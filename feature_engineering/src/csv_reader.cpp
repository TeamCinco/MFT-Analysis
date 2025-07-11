#include "csv_reader.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

double FastCSVReader::fast_atof(const char* str, const char** endptr) {
    double result = 0.0;
    double sign = 1.0;
    const char* p = str;
    
    if (*p == '-') {
        sign = -1.0;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    while (*p >= '0' && *p <= '9') {
        result = result * 10.0 + (*p - '0');
        p++;
    }
    
    if (*p == '.') {
        p++;
        double decimal_place = 0.1;
        while (*p >= '0' && *p <= '9') {
            result += (*p - '0') * decimal_place;
            decimal_place *= 0.1;
            p++;
        }
    }
    
    if (endptr) *endptr = p;
    return result * sign;
}

std::chrono::system_clock::time_point FastCSVReader::parse_timestamp(const std::string& datetime_str) {
    std::tm tm = {};
    std::istringstream ss(datetime_str);
    
    // Try different datetime formats
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        ss.clear();
        ss.str(datetime_str);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        tm.tm_hour = 6; // Default to 6 AM for daily data
    }
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::unique_ptr<OHLCVData> FastCSVReader::read_csv_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    auto data = std::make_unique<OHLCVData>();
    std::string line;
    
    // Skip header
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        const char* p = line.c_str();
        const char* end;
        
        // Parse symbol
        const char* symbol_start = p;
        while (*p != ',' && *p != '\0') p++;
        if (*p == '\0') continue;
        
        if (data->symbol.empty()) {
            data->symbol = std::string(symbol_start, p - symbol_start);
        }
        p++;
        
        // Parse datetime
        const char* datetime_start = p;
        while (*p != ',' && *p != '\0') p++;
        if (*p == '\0') continue;
        
        std::string datetime_str(datetime_start, p - datetime_start);
        try {
            data->timestamps.push_back(parse_timestamp(datetime_str));
        } catch (...) {
            // If parsing fails, use a sequential timestamp
            auto base_time = std::chrono::system_clock::now() - std::chrono::hours(24 * data->size());
            data->timestamps.push_back(base_time);
        }
        p++;
        
        // Parse OHLCV
        data->open.push_back(fast_atof(p, &end));
        p = end + 1;
        
        data->high.push_back(fast_atof(p, &end));
        p = end + 1;
        
        data->low.push_back(fast_atof(p, &end));
        p = end + 1;
        
        data->close.push_back(fast_atof(p, &end));
        p = end + 1;
        
        data->volume.push_back(fast_atof(p, &end));
    }
    
    return data;
}

std::vector<std::unique_ptr<OHLCVData>> FastCSVReader::read_directory(const std::string& directory) {
    std::vector<std::unique_ptr<OHLCVData>> all_data;
    
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".csv") {
            try {
                auto data = read_csv_file(entry.path().string());
                if (!data->empty()) {
                    std::cout << "Loaded " << data->symbol << ": " << data->size() << " rows" << std::endl;
                    all_data.push_back(std::move(data));
                }
            } catch (const std::exception& e) {
                std::cerr << "Error reading " << entry.path() << ": " << e.what() << std::endl;
            }
        }
    }
    
    return all_data;
}