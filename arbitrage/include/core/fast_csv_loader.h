#pragma once

#include "stock_data.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

class FastCSVLoader {
public:
    // Load all stock CSV files from a directory in parallel
    static std::vector<std::unique_ptr<StockData>> loadAllStocks(
        const std::string& data_directory,
        const PortfolioConstraints& constraints = PortfolioConstraints{}
    );
    
    // Load a single stock CSV file using memory-mapped I/O
    static std::unique_ptr<StockData> loadSingleStock(
        const std::string& csv_path
    );
    
    // Get list of all CSV files in directory
    static std::vector<std::string> getCSVFiles(const std::string& directory);
    
    // Performance metrics
    struct LoadingMetrics {
        size_t files_loaded = 0;
        size_t files_failed = 0;
        double loading_time_seconds = 0.0;
        double files_per_second = 0.0;
        size_t total_data_points = 0;
        size_t memory_used_mb = 0;
    };
    
    static LoadingMetrics getLastLoadingMetrics() { return last_metrics_; }
    
    // Helper function for fast string-to-double conversion (made public)
    static double fast_atof(const char* str, const char** endptr);
    
    // Filter stocks based on constraints
    static std::vector<std::unique_ptr<StockData>> filterStocks(
        std::vector<std::unique_ptr<StockData>> stocks,
        const PortfolioConstraints& constraints
    );

private:
    // Memory-mapped file reader
    class MemoryMappedFile {
    public:
        MemoryMappedFile(const std::string& filepath);
        ~MemoryMappedFile();
        
        const char* data() const { return data_; }
        size_t size() const { return size_; }
        bool isValid() const { return data_ != nullptr; }
        
    private:
        const char* data_ = nullptr;
        size_t size_ = 0;
        int fd_ = -1;
    };
    
    // Parse timestamp from string
    static std::chrono::system_clock::time_point parseTimestamp(
        const char* timestamp_str, const char** endptr
    );
    
    // Parse a single CSV line
    static bool parseCSVLine(
        const char* line_start, const char* line_end,
        StockData& stock_data
    );
    
    // Extract symbol from filename
    static std::string extractSymbolFromFilename(const std::string& filepath);
    
    // Filter stocks based on constraints
    static bool passesConstraints(
        const StockData& stock, 
        const PortfolioConstraints& constraints
    );
    
    // Classify stock by market cap and sector
    static void classifyStock(StockData& stock);
    
    // Performance tracking
    static LoadingMetrics last_metrics_;
    
    // Sector classification map
    static std::unordered_map<std::string, std::string> symbol_to_sector_;
    
    // Initialize sector mapping
    static void initializeSectorMapping();
};

// Parallel CSV loading utility
class ParallelCSVLoader {
public:
    static std::vector<std::unique_ptr<StockData>> loadInParallel(
        const std::vector<std::string>& csv_files,
        const PortfolioConstraints& constraints,
        unsigned int num_threads = 0  // 0 = auto-detect
    );
    
private:
    struct LoadingTask {
        std::vector<std::string> files;
        std::vector<std::unique_ptr<StockData>> results;
        size_t start_idx;
        size_t end_idx;
    };
    
    static void loadBatch(
        const std::vector<std::string>& files,
        size_t start_idx, size_t end_idx,
        const PortfolioConstraints& constraints,
        std::vector<std::unique_ptr<StockData>>& results
    );
};

// CSV column indices for your feature CSV format
enum class CSVColumn {
    DATETIME = 0,
    OPEN = 1,
    HIGH = 2,
    LOW = 3,
    CLOSE = 4,
    VOLUME = 5,
    SYMBOL = 6,
    DATA_FREQUENCY = 7,
    RETURNS = 8,
    SMA = 9,
    RSI = 10,
    VOLATILITY = 11,
    MOMENTUM = 12,
    // ... we only need the first few columns for arbitrage analysis
};

// Fast CSV parser optimized for your specific format
class OptimizedCSVParser {
public:
    // Parse your specific CSV format efficiently
    static std::unique_ptr<StockData> parseFeatureCSV(
        const char* csv_data, size_t data_size,
        const std::string& symbol
    );
    
private:
    // Skip to specific column in CSV line
    static const char* skipToColumn(const char* line_start, int column_index);
    
    // Parse CSV header to get column indices
    static std::unordered_map<std::string, int> parseHeader(const char* header_line);
    
    // Fast line counting for memory pre-allocation
    static size_t countLines(const char* data, size_t size);
};
