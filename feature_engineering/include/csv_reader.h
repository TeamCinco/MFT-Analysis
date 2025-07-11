#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>

struct OHLCVData {
    std::string symbol;
    std::vector<double> open, high, low, close, volume;
    std::vector<std::chrono::system_clock::time_point> timestamps;
    
    void reserve(size_t size) {
        open.reserve(size);
        high.reserve(size);
        low.reserve(size);
        close.reserve(size);
        volume.reserve(size);
        timestamps.reserve(size);
    }
    
    size_t size() const { return close.size(); }
    bool empty() const { return close.empty(); }
};

class FastCSVReader {
public:
    static std::unique_ptr<OHLCVData> read_csv_file(const std::string& filepath);
    static std::vector<std::unique_ptr<OHLCVData>> read_directory(const std::string& directory);
    
private:
    static double fast_atof(const char* str, const char** endptr);
    static std::chrono::system_clock::time_point parse_timestamp(const std::string& datetime_str);
};