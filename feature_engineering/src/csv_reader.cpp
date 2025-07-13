#include "../include/csv_reader.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

double FastCSVReader::fast_atof(const char* p, const char** endptr) {
    if (!p) { if (endptr) *endptr = p; return 0.0; }
    
    while (*p == ' ' || *p == '\t') p++;
    
    double sign = (*p == '-') ? (p++, -1.0) : 1.0;
    if (*p == '+') p++;

    double result = 0.0;
    while (*p >= '0' && *p <= '9') {
        result = result * 10.0 + (*p - '0');
        p++;
    }
    
    if (*p == '.') {
        p++;
        double f = 0.1;
        while (*p >= '0' && *p <= '9') {
            result += (*p - '0') * f;
            f *= 0.1;
            p++;
        }
    }
    if (endptr) *endptr = p;
    return result * sign;
}

std::chrono::system_clock::time_point FastCSVReader::parse_timestamp(const std::string& datetime_str) {
    std::tm tm = {};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        ss.clear();
        ss.str(datetime_str);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if(ss.fail()) return (std::chrono::system_clock::time_point::min)();
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::unique_ptr<OHLCVData> FastCSVReader::read_csv_file(const std::string& filepath) {
    // Cross-platform file reading approach
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (size == 0) {
        return std::make_unique<OHLCVData>();
    }
    
    // Read entire file into memory
    std::string content(size, '\0');
    if (!file.read(&content[0], size)) {
        throw std::runtime_error("Cannot read file: " + filepath);
    }
    file.close();
    
    auto data = std::make_unique<OHLCVData>();
    data->reserve(size / 80); // Estimate rows based on file size
    
    const char* ptr = content.c_str();
    const char* end = ptr + content.size();
    
    // Skip header line
    while (ptr < end && *ptr != '\n') ptr++;
    if (ptr < end) ptr++; // Skip the newline
    
    // Fast line-by-line parsing
    while (ptr < end) {
        const char* line_start = ptr;
        const char* line_end = ptr;
        
        // Find end of line
        while (line_end < end && *line_end != '\n' && *line_end != '\r') line_end++;
        
        if (line_end > line_start) {
            // Parse fields directly from memory
            const char* field_ptrs[7];
            int field_count = 0;
            const char* field_start = line_start;
            
            for (const char* p = line_start; p <= line_end && field_count < 7; p++) {
                if (p == line_end || *p == ',') {
                    field_ptrs[field_count++] = field_start;
                    field_start = p + 1;
                }
            }
            
            if (field_count >= 7) {
                // Extract symbol (first field)
                if (data->symbol.empty()) {
                    const char* symbol_end = field_ptrs[1] - 1;
                    data->symbol.assign(field_ptrs[0], symbol_end - field_ptrs[0]);
                }
                
                // Parse timestamp
                const char* ts_end = field_ptrs[2] - 1;
                std::string ts_str(field_ptrs[1], ts_end - field_ptrs[1]);
                data->timestamps.push_back(parse_timestamp(ts_str));
                
                // Parse OHLCV values directly
                const char* endptr;
                data->open.push_back(fast_atof(field_ptrs[2], &endptr));
                data->high.push_back(fast_atof(field_ptrs[3], &endptr));
                data->low.push_back(fast_atof(field_ptrs[4], &endptr));
                data->close.push_back(fast_atof(field_ptrs[5], &endptr));
                data->volume.push_back(fast_atof(field_ptrs[6], &endptr));
            }
        }
        
        // Move to next line, handling both \n and \r\n
        ptr = line_end;
        if (ptr < end && *ptr == '\r') ptr++;
        if (ptr < end && *ptr == '\n') ptr++;
    }
    
    return data;
}

std::vector<std::unique_ptr<OHLCVData>> FastCSVReader::read_directory(const std::string& directory) {
    std::vector<std::unique_ptr<OHLCVData>> all_data;
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error("Directory does not exist: " + directory);
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            try {
                auto data = read_csv_file(entry.path().string());
                if (data && !data->empty()) {
                    all_data.push_back(std::move(data));
                }
            } catch (const std::exception& e) {
                std::cerr << "Could not process file " << entry.path() << ": " << e.what() << std::endl;
            }
        }
    }
    return all_data;
}
