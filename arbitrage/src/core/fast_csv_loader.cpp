#include "fast_csv_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <chrono>

// Static member initialization
FastCSVLoader::LoadingMetrics FastCSVLoader::last_metrics_;
std::unordered_map<std::string, std::string> FastCSVLoader::symbol_to_sector_;

// Memory-mapped file implementation
FastCSVLoader::MemoryMappedFile::MemoryMappedFile(const std::string& filepath) {
    fd_ = open(filepath.c_str(), O_RDONLY);
    if (fd_ == -1) {
        return;
    }
    
    struct stat sb;
    if (fstat(fd_, &sb) == -1) {
        close(fd_);
        fd_ = -1;
        return;
    }
    
    size_ = sb.st_size;
    data_ = static_cast<const char*>(mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0));
    
    if (data_ == MAP_FAILED) {
        data_ = nullptr;
        close(fd_);
        fd_ = -1;
    }
}

FastCSVLoader::MemoryMappedFile::~MemoryMappedFile() {
    if (data_) {
        munmap(const_cast<char*>(data_), size_);
    }
    if (fd_ != -1) {
        close(fd_);
    }
}

std::vector<std::unique_ptr<StockData>> FastCSVLoader::loadAllStocks(
    const std::string& data_directory,
    const PortfolioConstraints& constraints) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Get all CSV files
    auto csv_files = getCSVFiles(data_directory);
    
    // Load in parallel
    auto stocks = ParallelCSVLoader::loadInParallel(csv_files, constraints);
    
    // Filter stocks based on constraints
    stocks = filterStocks(std::move(stocks), constraints);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Update metrics
    last_metrics_.files_loaded = stocks.size();
    last_metrics_.files_failed = csv_files.size() - stocks.size();
    last_metrics_.loading_time_seconds = duration.count() / 1000.0;
    last_metrics_.files_per_second = stocks.size() / last_metrics_.loading_time_seconds;
    
    size_t total_points = 0;
    for (const auto& stock : stocks) {
        total_points += stock->size();
    }
    last_metrics_.total_data_points = total_points;
    last_metrics_.memory_used_mb = (total_points * sizeof(double) * 6) / (1024 * 1024); // Rough estimate
    
    return stocks;
}

std::unique_ptr<StockData> FastCSVLoader::loadSingleStock(const std::string& csv_path) {
    MemoryMappedFile file(csv_path);
    if (!file.isValid()) {
        return nullptr;
    }
    
    std::string symbol = extractSymbolFromFilename(csv_path);
    return OptimizedCSVParser::parseFeatureCSV(file.data(), file.size(), symbol);
}

std::vector<std::string> FastCSVLoader::getCSVFiles(const std::string& directory) {
    std::vector<std::string> csv_files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                csv_files.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory " << directory << ": " << e.what() << std::endl;
    }
    
    return csv_files;
}

double FastCSVLoader::fast_atof(const char* str, const char** endptr) {
    // Simple fast implementation - can be optimized further
    double result = 0.0;
    double sign = 1.0;
    const char* p = str;
    
    // Skip whitespace
    while (*p == ' ' || *p == '\t') p++;
    
    // Handle sign
    if (*p == '-') {
        sign = -1.0;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Parse integer part
    while (*p >= '0' && *p <= '9') {
        result = result * 10.0 + (*p - '0');
        p++;
    }
    
    // Parse decimal part
    if (*p == '.') {
        p++;
        double fraction = 0.1;
        while (*p >= '0' && *p <= '9') {
            result += (*p - '0') * fraction;
            fraction *= 0.1;
            p++;
        }
    }
    
    if (endptr) {
        *endptr = p;
    }
    
    return result * sign;
}

std::string FastCSVLoader::extractSymbolFromFilename(const std::string& filepath) {
    std::filesystem::path path(filepath);
    std::string filename = path.stem().string();
    
    // Remove "_features" suffix if present
    size_t pos = filename.find("_features");
    if (pos != std::string::npos) {
        filename = filename.substr(0, pos);
    }
    
    return filename;
}

std::vector<std::unique_ptr<StockData>> FastCSVLoader::filterStocks(
    std::vector<std::unique_ptr<StockData>> stocks,
    const PortfolioConstraints& constraints) {
    
    std::vector<std::unique_ptr<StockData>> filtered;
    
    for (auto& stock : stocks) {
        if (stock && passesConstraints(*stock, constraints)) {
            filtered.push_back(std::move(stock));
        }
    }
    
    return filtered;
}

bool FastCSVLoader::passesConstraints(
    const StockData& stock,
    const PortfolioConstraints& constraints) {
    
    if (stock.empty()) {
        return false;
    }
    
    // Check price constraints
    double current_price = stock.close.back();
    if (current_price < constraints.min_stock_price || 
        current_price > constraints.max_stock_price) {
        return false;
    }
    
    // Check minimum data points
    if (stock.size() < 100) { // Minimum for statistical analysis
        return false;
    }
    
    return true;
}

// Parallel CSV loader implementation
std::vector<std::unique_ptr<StockData>> ParallelCSVLoader::loadInParallel(
    const std::vector<std::string>& csv_files,
    const PortfolioConstraints& constraints,
    unsigned int num_threads) {
    
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
    }
    
    std::vector<std::unique_ptr<StockData>> all_results(csv_files.size());
    std::vector<std::thread> threads;
    
    const size_t batch_size = (csv_files.size() + num_threads - 1) / num_threads;
    
    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t start_idx = t * batch_size;
        size_t end_idx = std::min(start_idx + batch_size, csv_files.size());
        
        if (start_idx >= csv_files.size()) break;
        
        threads.emplace_back([&csv_files, &all_results, &constraints, start_idx, end_idx]() {
            for (size_t i = start_idx; i < end_idx; ++i) {
                try {
                    all_results[i] = FastCSVLoader::loadSingleStock(csv_files[i]);
                } catch (const std::exception& e) {
                    std::cerr << "Error loading " << csv_files[i] << ": " << e.what() << std::endl;
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Remove null entries
    std::vector<std::unique_ptr<StockData>> results;
    for (auto& stock : all_results) {
        if (stock) {
            results.push_back(std::move(stock));
        }
    }
    
    return results;
}

// Optimized CSV parser implementation
std::unique_ptr<StockData> OptimizedCSVParser::parseFeatureCSV(
    const char* csv_data, size_t data_size, const std::string& symbol) {
    
    auto stock = std::make_unique<StockData>();
    stock->symbol = symbol;
    
    // Count lines for pre-allocation
    size_t line_count = countLines(csv_data, data_size);
    if (line_count < 2) return nullptr; // Need at least header + 1 data line
    
    stock->reserve(line_count - 1); // Exclude header
    
    const char* line_start = csv_data;
    const char* data_end = csv_data + data_size;
    
    // Skip header line
    while (line_start < data_end && *line_start != '\n') {
        line_start++;
    }
    if (line_start < data_end) line_start++; // Skip the newline
    
    // Parse data lines
    while (line_start < data_end) {
        const char* line_end = line_start;
        while (line_end < data_end && *line_end != '\n' && *line_end != '\r') {
            line_end++;
        }
        
        if (line_end > line_start) {
            // Parse this line - simplified version
            const char* p = line_start;
            
            // Skip datetime (column 0)
            p = skipToColumn(p, 1);
            
            // Parse OHLCV data (columns 1-5)
            if (p < line_end) {
                const char* endptr;
                double open = FastCSVLoader::fast_atof(p, &endptr);
                stock->open.push_back(open);
                p = skipToColumn(endptr, 1);
                
                double high = FastCSVLoader::fast_atof(p, &endptr);
                stock->high.push_back(high);
                p = skipToColumn(endptr, 1);
                
                double low = FastCSVLoader::fast_atof(p, &endptr);
                stock->low.push_back(low);
                p = skipToColumn(endptr, 1);
                
                double close = FastCSVLoader::fast_atof(p, &endptr);
                stock->close.push_back(close);
                p = skipToColumn(endptr, 1);
                
                double volume = FastCSVLoader::fast_atof(p, &endptr);
                stock->volume.push_back(volume);
            }
        }
        
        // Move to next line
        line_start = line_end;
        while (line_start < data_end && (*line_start == '\n' || *line_start == '\r')) {
            line_start++;
        }
    }
    
    // Calculate returns and statistics
    stock->calculateReturns();
    stock->calculateStatistics();
    
    return stock;
}

const char* OptimizedCSVParser::skipToColumn(const char* line_start, int column_index) {
    const char* p = line_start;
    for (int i = 0; i < column_index && *p; ++i) {
        while (*p && *p != ',' && *p != '\n' && *p != '\r') p++;
        if (*p == ',') p++;
    }
    return p;
}

size_t OptimizedCSVParser::countLines(const char* data, size_t size) {
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            count++;
        }
    }
    return count;
}
