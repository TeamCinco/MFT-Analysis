#pragma once
#include <string>

struct StockData {
    float datetime_index;
    float open, high, low, close, volume;
    std::string symbol;
    float returns, sma, rsi, volatility, momentum;
    float spread, internal_bar_strength, skewness_30, kurtosis_30;
    float log_pct_change_5, auto_correlation_50_10, kama_10_2_30;
    float linear_slope_20, linear_slope_60, parkinson_volatility_20;
    float volume_sma_20, velocity, acceleration;
    float candle_way, candle_filling, candle_amplitude;
    std::string date_string;
};