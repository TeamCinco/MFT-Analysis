#include "csv_reader.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

double FastCSVReader::fast_atof(const char* str, const char** endptr) {
    if (!str) {
        if (endptr) *endptr = str;
        return 0.0;
    }
    
    double result = 0.0;
    double sign = 1.0;
    const char* p = str;
    
    // Skip whitespace
    while (*p == ' ' || *p == '\t') p++;
    
    if (*p == '-') {
        sign = -1.0;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Handle empty or invalid strings
    if (*p < '0' || *p > '9') {
        if (endptr) *endptr = str;
        return 0.0;
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
        if (ss.fail()) {
            // If all parsing fails, return current time
            return std::chrono::system_clock::now();
        }
        tm.tm_hour = 9; // Default to 9 AM for daily data
        tm.tm_min = 30;
        tm.tm_sec = 0;
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
    size_t line_count = 0;
    
    // Skip header
    if (std::getline(file, line)) {
        line_count++;
    }
    
    // Reserve space for better performance
    data->reserve(10000);
    
    while (std::getline(file, line)) {
        line_count++;
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        
        try {
            // Parse CSV line safely
            std::vector<std::string> fields;
            std::stringstream ss(line);
            std::string field;
            
            while (std::getline(ss, field, ',')) {
                fields.push_back(field);
            }
            
            if (fields.size() < 6) {
                continue; // Skip silently
            }
            
            // Extract symbol
            if (data->symbol.empty() && !fields[0].empty()) {
                data->symbol = fields[0];
            }
            
            // Parse timestamp
            try {
                data->timestamps.push_back(parse_timestamp(fields[1]));
            } catch (...) {
                // Use sequential timestamp if parsing fails
                auto base_time = std::chrono::system_clock::now() - std::chrono::hours(24 * data->size());
                data->timestamps.push_back(base_time);
            }
            
            // Parse OHLCV with error checking
            const char* endptr;
            
            double open_val = fast_atof(fields[2].c_str(), &endptr);
            if (endptr == fields[2].c_str()) open_val = 0.0;
            data->open.push_back(open_val);
            
            double high_val = fast_atof(fields[3].c_str(), &endptr);
            if (endptr == fields[3].c_str()) high_val = open_val;
            data->high.push_back(high_val);
            
            double low_val = fast_atof(fields[4].c_str(), &endptr);
            if (endptr == fields[4].c_str()) low_val = open_val;
            data->low.push_back(low_val);
            
            double close_val = fast_atof(fields[5].c_str(), &endptr);
            if (endptr == fields[5].c_str()) close_val = open_val;
            data->close.push_back(close_val);
            
            double volume_val = 0.0;
            if (fields.size() > 6) {
                volume_val = fast_atof(fields[6].c_str(), &endptr);
                if (endptr == fields[6].c_str()) volume_val = 1000.0; // Default volume
            }
            data->volume.push_back(volume_val);
            
        } catch (...) {
            continue; // Skip silently
        }
    }
    
    file.close();
    
    if (data->empty()) {
        throw std::runtime_error("No valid data found in file: " + filepath);
    }
    
    return data;
}

std::vector<std::unique_ptr<OHLCVData>> FastCSVReader::read_directory(const std::string& directory) {
    std::vector<std::unique_ptr<OHLCVData>> all_data;
    
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error("Directory does not exist: " + directory);
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".csv") {
            try {
                auto data = read_csv_file(entry.path().string());
                if (!data->empty()) {
                    all_data.push_back(std::move(data));
                }
            } catch (...) {
                // Skip silently
            }
        }
    }
    
    if (all_data.empty()) {
        throw std::runtime_error("No valid CSV files found in directory: " + directory);
    }
    
    return all_data;
}