#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <chrono>

struct OHLCVData; // Forward declaration

class FastCSVWriter {
public:
    FastCSVWriter() = default;
    ~FastCSVWriter() = default;
    
    // Write OHLCV data with calculated features in wide format
    static void write_ohlcv_with_features(
        const std::string& filepath,
        const OHLCVData& ohlcv_data,
        const std::vector<double>& returns,
        const std::vector<double>& sma,
        const std::vector<double>& rsi,
        const std::vector<double>& volatility,
        const std::vector<double>& momentum,
        const std::vector<double>& spread,
        const std::vector<double>& internal_bar_strength,
        const std::vector<double>& skewness_30,
        const std::vector<double>& kurtosis_30,
        const std::vector<double>& log_pct_change_5,
        const std::vector<double>& auto_correlation_50_10,
        const std::vector<double>& kama_10_2_30,
        const std::vector<double>& linear_slope_20,
        const std::vector<double>& linear_slope_60,
        const std::vector<double>& parkinson_volatility_20,
        const std::vector<double>& volume_sma_20,
        const std::vector<double>& velocity,
        const std::vector<double>& acceleration,
        const std::vector<int>& candle_way,
        const std::vector<double>& candle_filling,
        const std::vector<double>& candle_amplitude,
        const std::string& data_frequency = "daily"
    );
    
private:
    static std::string format_datetime(const std::chrono::system_clock::time_point& tp);
    static std::string format_double(double value, int precision = 6);
    static std::string safe_get_value(const std::vector<double>& vec, size_t index);
    static std::string safe_get_int_value(const std::vector<int>& vec, size_t index);
};