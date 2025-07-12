#pragma once

#include "ohlcv_data.h"
#include <string>
#include <vector>
#include <memory>

class FastCSVReader {
public:
    // Reads a single CSV file into an OHLCVData structure.
    static std::unique_ptr<OHLCVData> read_csv_file(const std::string& filepath);

    // Reads all CSV files in a directory.
    static std::vector<std::unique_ptr<OHLCVData>> read_directory(const std::string& directory);

private:
    // A faster, non-locale-dependent string-to-double converter.
    static double fast_atof(const char* str, const char** endptr);

    // Parses common timestamp formats.
    static std::chrono::system_clock::time_point parse_timestamp(const std::string& datetime_str);
};