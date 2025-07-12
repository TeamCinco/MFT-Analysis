#include "../include/csv_writer.h"
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>

std::string FastCSVWriter::format_datetime(const std::chrono::system_clock::time_point& tp) {
    if (tp == std::chrono::system_clock::time_point::min()) return "";
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
    #ifdef _WIN32
    localtime_s(&tm, &time_t);
    #else
    localtime_r(&time_t, &tm);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string FastCSVWriter::format_double(double value, int precision) {
    if (std::isnan(value) || std::isinf(value)) return "";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string FastCSVWriter::safe_get_value(const std::vector<double>& vec, size_t index, size_t offset) {
    size_t effective_index = index >= offset ? index - offset : -1;
    return (effective_index < vec.size()) ? format_double(vec[effective_index]) : "";
}

std::string FastCSVWriter::safe_get_int_value(const std::vector<int>& vec, size_t index, size_t offset) {
    size_t effective_index = index >= offset ? index - offset : -1;
    return (effective_index < vec.size()) ? std::to_string(vec[effective_index]) : "";
}

void FastCSVWriter::write_ohlcv_with_features(
    const std::string& filepath, const OHLCVData& ohlcv_data,
    const FeatureSet& features, const std::string& data_frequency) {
    try {
        if (auto p = std::filesystem::path(filepath).parent_path(); !p.empty()) {
            std::filesystem::create_directories(p);
        }
        
        // Use large buffer for maximum write performance
        constexpr size_t BUFFER_SIZE = 1024 * 1024; // 1MB buffer
        std::vector<char> buffer;
        buffer.reserve(ohlcv_data.size() * 200); // Estimate line length
        
        // Build entire CSV content in memory first
        std::string content;
        content.reserve(ohlcv_data.size() * 200);
        
        // Header
        content += "datetime,open,high,low,close,volume,symbol,data_frequency,returns,sma,rsi,volatility,momentum,spread,internal_bar_strength,skewness_30,kurtosis_30,log_pct_change_5,auto_correlation_50_10,kama_10_2_30,linear_slope_20,linear_slope_60,parkinson_volatility_20,volume_sma_20,velocity,acceleration,candle_way,candle_filling,candle_amplitude\n";
        
        // Pre-allocate string for reuse
        std::string line;
        line.reserve(200);
        
        for (size_t i = 0; i < ohlcv_data.size(); ++i) {
            line.clear();
            
            // Build line efficiently
            line += format_datetime(ohlcv_data.timestamps[i]) + ",";
            line += format_double(ohlcv_data.open[i]) + ",";
            line += format_double(ohlcv_data.high[i]) + ",";
            line += format_double(ohlcv_data.low[i]) + ",";
            line += format_double(ohlcv_data.close[i]) + ",";
            line += format_double(ohlcv_data.volume[i], 0) + ",";
            line += ohlcv_data.symbol + "," + data_frequency + ",";
            line += safe_get_value(features.returns, i, 1) + ",";
            line += safe_get_value(features.sma, i, 19) + ",";
            line += safe_get_value(features.rsi, i, 14) + ",";
            line += safe_get_value(features.volatility, i, 20) + ",";
            line += safe_get_value(features.momentum, i, 10) + ",";
            line += safe_get_value(features.spread, i, 0) + ",";
            line += safe_get_value(features.internal_bar_strength, i, 0) + ",";
            line += safe_get_value(features.skewness_30, i, 29) + ",";
            line += safe_get_value(features.kurtosis_30, i, 29) + ",";
            line += safe_get_value(features.log_pct_change_5, i, 5) + ",";
            line += safe_get_value(features.auto_correlation_50_10, i, 59) + ",";
            line += safe_get_value(features.kama_10_2_30, i, 0) + ",";
            line += safe_get_value(features.linear_slope_20, i, 19) + ",";
            line += safe_get_value(features.linear_slope_60, i, 59) + ",";
            line += safe_get_value(features.parkinson_volatility_20, i, 19) + ",";
            line += safe_get_value(features.volume_sma_20, i, 19) + ",";
            line += safe_get_value(features.velocity, i, 1) + ",";
            line += safe_get_value(features.acceleration, i, 2) + ",";
            line += safe_get_int_value(features.candle_way, i, 0) + ",";
            line += safe_get_value(features.candle_filling, i, 0) + ",";
            line += safe_get_value(features.candle_amplitude, i, 0) + "\n";
            
            content += line;
        }
        
        // Single write operation for maximum speed
        std::ofstream file(filepath, std::ios::out | std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("Cannot create file: " + filepath);
        
        // Set large buffer for file stream
        file.rdbuf()->pubsetbuf(nullptr, BUFFER_SIZE);
        file.write(content.data(), content.size());
        file.close();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error writing CSV file: " + std::string(e.what()));
    }
}
