#include "technical_indicators.h"
#include "simd_utils.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <iostream>  


std::vector<double> TechnicalIndicators::calculate_returns(const std::vector<double>& prices) {
    std::vector<double> returns;
    if (prices.size() < 2) return returns;
    
    returns.reserve(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        if (prices[i-1] != 0.0) {
            returns.push_back((prices[i] - prices[i-1]) / prices[i-1]);
        } else {
            returns.push_back(0.0);
        }
    }
    return returns;
}

std::vector<double> TechnicalIndicators::simple_moving_average(const std::vector<double>& data, size_t window) {
    if (data.empty() || window == 0 || window > data.size()) {
        return std::vector<double>();
    }
    
    std::vector<double> result;
    result.reserve(data.size() - window + 1);
    
    // Calculate first window
    double sum = 0.0;
    for (size_t i = 0; i < window; ++i) {
        sum += data[i];
    }
    result.push_back(sum / window);
    
    // Rolling calculation
    for (size_t i = window; i < data.size(); ++i) {
        sum = sum - data[i - window] + data[i];
        result.push_back(sum / window);
    }
    
    return result;
}

std::vector<double> TechnicalIndicators::calculate_rsi(const std::vector<double>& prices, int period) {
    if (prices.size() < static_cast<size_t>(period + 1)) {
        return std::vector<double>();
    }
    
    std::vector<double> gains, losses;
    gains.reserve(prices.size() - 1);
    losses.reserve(prices.size() - 1);
    
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }
    
    auto avg_gains = simple_moving_average(gains, period);
    auto avg_losses = simple_moving_average(losses, period);
    
    std::vector<double> rsi;
    rsi.reserve(avg_gains.size());
    
    for (size_t i = 0; i < avg_gains.size(); ++i) {
        if (avg_losses[i] == 0) {
            rsi.push_back(100);
        } else {
            double rs = avg_gains[i] / avg_losses[i];
            rsi.push_back(100 - (100 / (1 + rs)));
        }
    }
    return rsi;
}

std::vector<double> TechnicalIndicators::calculate_rolling_volatility(const std::vector<double>& returns, int window) {
    std::vector<double> volatility;
    if (returns.size() < static_cast<size_t>(window) || window <= 1) {
        return volatility;
    }
    
    volatility.reserve(returns.size() - window + 1);
    
    for (size_t i = static_cast<size_t>(window); i <= returns.size(); ++i) {
        double mean = 0.0;
        for (size_t j = i - window; j < i; ++j) {
            mean += returns[j];
        }
        mean /= window;
        
        double var = 0.0;
        for (size_t j = i - window; j < i; ++j) {
            var += (returns[j] - mean) * (returns[j] - mean);
        }
        volatility.push_back(std::sqrt(var / (window - 1)));
    }
    return volatility;
}

std::vector<double> TechnicalIndicators::calculate_momentum(const std::vector<double>& prices, int period) {
    std::vector<double> momentum;
    if (prices.size() <= static_cast<size_t>(period)) {
        return momentum;
    }
    
    momentum.reserve(prices.size() - period);
    
    for (size_t i = static_cast<size_t>(period); i < prices.size(); ++i) {
        if (prices[i - period] != 0.0) {
            momentum.push_back((prices[i] - prices[i - period]) / prices[i - period]);
        } else {
            momentum.push_back(0.0);
        }
    }
    return momentum;
}

std::vector<double> TechnicalIndicators::compute_spread(const std::vector<double>& high, const std::vector<double>& low) {
    std::vector<double> spread;
    if (high.size() != low.size()) {
        return spread;
    }
    
    spread.reserve(high.size());
    for (size_t i = 0; i < high.size(); ++i) {
        spread.push_back(high[i] - low[i]);
    }
    return spread;
}

std::vector<double> TechnicalIndicators::internal_bar_strength(
    const std::vector<double>& open, const std::vector<double>& high,
    const std::vector<double>& low, const std::vector<double>& close) {
    
    std::vector<double> ibs;
    if (open.size() != high.size() || high.size() != low.size() || low.size() != close.size()) {
        return ibs;
    }
    
    ibs.reserve(close.size());
    for (size_t i = 0; i < close.size(); ++i) {
        double range = high[i] - low[i];
        if (range > 0) {
            ibs.push_back((close[i] - low[i]) / range);
        } else {
            ibs.push_back(0.5);  // Neutral when no range
        }
    }
    return ibs;
}

// Simplified versions of complex indicators to avoid memory issues
std::pair<std::vector<int>, std::pair<std::vector<double>, std::vector<double>>> 
TechnicalIndicators::candle_information(const std::vector<double>& open, const std::vector<double>& high,
                                       const std::vector<double>& low, const std::vector<double>& close) {
    std::vector<int> candle_way;
    std::vector<double> filling, amplitude;
    
    if (open.size() != high.size() || high.size() != low.size() || low.size() != close.size()) {
        return std::make_pair(candle_way, std::make_pair(filling, amplitude));
    }
    
    candle_way.reserve(open.size());
    filling.reserve(open.size());
    amplitude.reserve(open.size());
    
    for (size_t i = 0; i < open.size(); ++i) {
        // Candle way: 1 for bullish, -1 for bearish, 0 for doji
        if (close[i] > open[i]) {
            candle_way.push_back(1);
        } else if (close[i] < open[i]) {
            candle_way.push_back(-1);
        } else {
            candle_way.push_back(0);
        }
        
        // Filling: body size relative to total range
        double body_size = std::abs(close[i] - open[i]);
        double total_range = high[i] - low[i];
        filling.push_back(total_range > 0 ? body_size / total_range : 0.0);
        
        // Amplitude: total range
        amplitude.push_back(total_range);
    }
    
    return std::make_pair(candle_way, std::make_pair(filling, amplitude));
}

// Add stub implementations for other functions to avoid linking errors
std::pair<std::vector<double>, std::vector<double>> TechnicalIndicators::derivatives(const std::vector<double>& prices) {
    std::vector<double> velocity, acceleration;
    if (prices.size() < 2) return std::make_pair(velocity, acceleration);
    
    velocity.reserve(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        velocity.push_back(prices[i] - prices[i-1]);
    }
    
    if (velocity.size() < 2) return std::make_pair(velocity, acceleration);
    
    acceleration.reserve(velocity.size() - 1);
    for (size_t i = 1; i < velocity.size(); ++i) {
        acceleration.push_back(velocity[i] - velocity[i-1]);
    }
    
    return std::make_pair(velocity, acceleration);
}

// Stub implementations for complex indicators
std::vector<double> TechnicalIndicators::log_pct_change(const std::vector<double>& prices, int window_size) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::auto_correlation(const std::vector<double>& prices, int window_size, int lag) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::skewness(const std::vector<double>& prices, int window_size) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::kurtosis(const std::vector<double>& prices, int window_size) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::kama(const std::vector<double>& prices, int l1, int l2, int l3) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::linear_slope(const std::vector<double>& prices, int window_size) {
    return std::vector<double>(prices.size(), 0.0);
}

std::vector<double> TechnicalIndicators::close_to_close_volatility(const std::vector<double>& prices, int window_size) {
    auto returns = calculate_returns(prices);
    return calculate_rolling_volatility(returns, window_size);
}

std::vector<double> TechnicalIndicators::parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size) {
    return std::vector<double>(high.size(), 0.0);
}