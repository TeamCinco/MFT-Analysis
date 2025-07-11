#include "csv_writer.h"
#include "csv_reader.h"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <stdexcept>

std::string FastCSVWriter::format_datetime(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm = std::localtime(&time_t);
    
    if (!tm) {
        return "1970-01-01 00:00:00"; // Fallback
    }
    
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string FastCSVWriter::format_double(double value, int precision) {
    if (std::isnan(value) || std::isinf(value)) {
        return "";
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string FastCSVWriter::safe_get_value(const std::vector<double>& vec, size_t index) {
    if (index < vec.size()) {
        return format_double(vec[index]);
    }
    return "";
}

std::string FastCSVWriter::safe_get_int_value(const std::vector<int>& vec, size_t index) {
    if (index < vec.size()) {
        return std::to_string(vec[index]);
    }
    return "";
}

void FastCSVWriter::write_ohlcv_with_features(
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
    const std::string& data_frequency
) {
    try {
        std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
        
        std::ofstream file(filepath, std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create file: " + filepath);
        }
        
        // Write header
        file << "datetime,open,high,low,close,volume,symbol,data_frequency,returns,sma,rsi,volatility,momentum,spread,internal_bar_strength,skewness_30,kurtosis_30,log_pct_change_5,auto_correlation_50_10,kama_10_2_30,linear_slope_20,linear_slope_60,parkinson_volatility_20,volume_sma_20,velocity,acceleration,candle_way,candle_filling,candle_amplitude\n";
        
        // Write data rows
        for (size_t i = 0; i < ohlcv_data.size(); ++i) {
            // Basic OHLCV data
            if (i < ohlcv_data.timestamps.size()) {
                file << format_datetime(ohlcv_data.timestamps[i]);
            }
            file << ",";
            
            file << format_double(ohlcv_data.open[i]) << ",";
            file << format_double(ohlcv_data.high[i]) << ",";
            file << format_double(ohlcv_data.low[i]) << ",";
            file << format_double(ohlcv_data.close[i]) << ",";
            file << format_double(ohlcv_data.volume[i]) << ",";
            file << ohlcv_data.symbol << ",";
            file << data_frequency << ",";
            
            // Technical indicators with proper indexing
            file << safe_get_value(returns, i > 0 ? i - 1 : 0) << ",";
            file << safe_get_value(sma, i) << ",";
            file << safe_get_value(rsi, i) << ",";
            file << safe_get_value(volatility, i) << ",";
            file << safe_get_value(momentum, i) << ",";
            file << safe_get_value(spread, i) << ",";
            file << safe_get_value(internal_bar_strength, i) << ",";
            file << safe_get_value(skewness_30, i) << ",";
            file << safe_get_value(kurtosis_30, i) << ",";
            file << safe_get_value(log_pct_change_5, i) << ",";
            file << safe_get_value(auto_correlation_50_10, i) << ",";
            file << safe_get_value(kama_10_2_30, i) << ",";
            file << safe_get_value(linear_slope_20, i) << ",";
            file << safe_get_value(linear_slope_60, i) << ",";
            file << safe_get_value(parkinson_volatility_20, i) << ",";
            file << safe_get_value(volume_sma_20, i) << ",";
            file << safe_get_value(velocity, i > 0 ? i - 1 : 0) << ",";
            file << safe_get_value(acceleration, i > 1 ? i - 2 : 0) << ",";
            file << safe_get_int_value(candle_way, i) << ",";
            file << safe_get_value(candle_filling, i) << ",";
            file << safe_get_value(candle_amplitude, i);
            
            file << "\n";
        }
        
        file.close();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error writing CSV file: " + std::string(e.what()));
    }
}