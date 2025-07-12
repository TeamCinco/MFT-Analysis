#pragma once

#include "ohlcv_data.h"
#include "batch_ohlc_processor.h" // For FeatureSet
#include <string>

class FastCSVWriter {
public:
    // Writes the original OHLCV data along with all calculated features to a single wide-format CSV file.
    static void write_ohlcv_with_features(
        const std::string& filepath,
        const OHLCVData& ohlcv_data,
        const FeatureSet& features,
        const std::string& data_frequency = "daily"
    );

private:
    static std::string format_datetime(const std::chrono::system_clock::time_point& tp);
    static std::string format_double(double value, int precision = 6);
    static std::string safe_get_value(const std::vector<double>& vec, size_t index, size_t offset = 0);
    static std::string safe_get_int_value(const std::vector<int>& vec, size_t index, size_t offset = 0);
};