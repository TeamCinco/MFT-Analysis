#pragma once

#include <vector>
#include <string>
#include <chrono>

// Holds the raw OHLCV data for a single financial instrument.
struct OHLCVData {
    std::string symbol;
    std::vector<std::chrono::system_clock::time_point> timestamps;
    std::vector<double> open, high, low, close, volume;

    void reserve(size_t size) {
        timestamps.reserve(size);
        open.reserve(size);
        high.reserve(size);
        low.reserve(size);
        close.reserve(size);
        volume.reserve(size);
    }

    size_t size() const {
        return close.size();
    }
    
    bool empty() const {
        return close.empty();
    }
};

// Holds the set of calculated features for a single financial instrument.
struct FeatureSet {
    std::vector<double> returns;
    std::vector<double> sma;
    std::vector<double> rsi;
    std::vector<double> volatility;
    std::vector<double> momentum;
    std::vector<double> spread;
    std::vector<double> internal_bar_strength;
    std::vector<double> skewness_30;
    std::vector<double> kurtosis_30;
    std::vector<double> log_pct_change_5;
    std::vector<double> auto_correlation_50_10;
    std::vector<double> kama_10_2_30;
    std::vector<double> linear_slope_20;
    std::vector<double> linear_slope_60;
    std::vector<double> parkinson_volatility_20;
    std::vector<double> volume_sma_20;
    std::vector<double> velocity;
    std::vector<double> acceleration;
    std::vector<int>    candle_way;
    std::vector<double> candle_filling;
    std::vector<double> candle_amplitude;
};