#pragma once
#include <vector>
#include <utility>

class TechnicalIndicators {
public:
    // Core indicators
    static std::vector<double> calculate_returns(const std::vector<double>& prices);
    static std::vector<double> calculate_rsi(const std::vector<double>& prices, int period);
    static std::vector<double> simple_moving_average(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_rolling_volatility(const std::vector<double>& returns, int window);
    static std::vector<double> calculate_momentum(const std::vector<double>& prices, int period);
    
    // Candle information
    static std::pair<std::vector<int>, std::pair<std::vector<double>, std::vector<double>>> 
           candle_information(const std::vector<double>& open, const std::vector<double>& high,
                             const std::vector<double>& low, const std::vector<double>& close);
    static std::vector<double> compute_spread(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> internal_bar_strength(const std::vector<double>& open, const std::vector<double>& high,
                                                    const std::vector<double>& low, const std::vector<double>& close);
    
    // Math features
    static std::pair<std::vector<double>, std::vector<double>> derivatives(const std::vector<double>& prices);
    static std::vector<double> log_pct_change(const std::vector<double>& prices, int window_size);
    static std::vector<double> auto_correlation(const std::vector<double>& prices, int window_size, int lag);
    static std::vector<double> skewness(const std::vector<double>& prices, int window_size);
    static std::vector<double> kurtosis(const std::vector<double>& prices, int window_size);
    
    // Trend indicators
    static std::vector<double> kama(const std::vector<double>& prices, int l1, int l2, int l3);
    static std::vector<double> linear_slope(const std::vector<double>& prices, int window_size);
    
    // Volatility indicators
    static std::vector<double> close_to_close_volatility(const std::vector<double>& prices, int window_size);
    static std::vector<double> parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size);
};