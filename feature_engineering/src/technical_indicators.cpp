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

std::vector<double> TechnicalIndicators::log_pct_change(const std::vector<double>& prices, int window_size) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(window_size + 1)) return result;
    
    result.reserve(prices.size() - window_size);
    for (size_t i = window_size; i < prices.size(); ++i) {
        if (prices[i - window_size] > 0) {
            result.push_back(std::log(prices[i] / prices[i - window_size]));
        } else {
            result.push_back(0.0);
        }
    }
    return result;
}

std::vector<double> TechnicalIndicators::auto_correlation(const std::vector<double>& prices, int window_size, int lag) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(window_size + lag)) return result;
    
    result.reserve(prices.size() - window_size - lag + 1);
    
    for (size_t i = window_size + lag - 1; i < prices.size(); ++i) {
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
        
        for (int j = 0; j < window_size; ++j) {
            double x = prices[i - window_size + 1 + j - lag];
            double y = prices[i - window_size + 1 + j];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        }
        
        double numerator = window_size * sum_xy - sum_x * sum_y;
        double denominator = std::sqrt((window_size * sum_x2 - sum_x * sum_x) * 
                                     (window_size * sum_y2 - sum_y * sum_y));
        
        if (denominator != 0) {
            result.push_back(numerator / denominator);
        } else {
            result.push_back(0.0);
        }
    }
    return result;
}

std::vector<double> TechnicalIndicators::skewness(const std::vector<double>& prices, int window_size) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(window_size)) return result;
    
    result.reserve(prices.size() - window_size + 1);
    
    for (size_t i = window_size - 1; i < prices.size(); ++i) {
        double mean = 0.0;
        for (int j = 0; j < window_size; ++j) {
            mean += prices[i - window_size + 1 + j];
        }
        mean /= window_size;
        
        double m2 = 0.0, m3 = 0.0;
        for (int j = 0; j < window_size; ++j) {
            double diff = prices[i - window_size + 1 + j] - mean;
            m2 += diff * diff;
            m3 += diff * diff * diff;
        }
        m2 /= window_size;
        m3 /= window_size;
        
        double std_dev = std::sqrt(m2);
        if (std_dev > 0) {
            result.push_back(m3 / (std_dev * std_dev * std_dev));
        } else {
            result.push_back(0.0);
        }
    }
    return result;
}

std::vector<double> TechnicalIndicators::kurtosis(const std::vector<double>& prices, int window_size) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(window_size)) return result;
    
    result.reserve(prices.size() - window_size + 1);
    
    for (size_t i = window_size - 1; i < prices.size(); ++i) {
        double mean = 0.0;
        for (int j = 0; j < window_size; ++j) {
            mean += prices[i - window_size + 1 + j];
        }
        mean /= window_size;
        
        double m2 = 0.0, m4 = 0.0;
        for (int j = 0; j < window_size; ++j) {
            double diff = prices[i - window_size + 1 + j] - mean;
            double diff2 = diff * diff;
            m2 += diff2;
            m4 += diff2 * diff2;
        }
        m2 /= window_size;
        m4 /= window_size;
        
        if (m2 > 0) {
            result.push_back((m4 / (m2 * m2)) - 3.0);  // Excess kurtosis
        } else {
            result.push_back(0.0);
        }
    }
    return result;
}

std::vector<double> TechnicalIndicators::kama(const std::vector<double>& prices, int l1, int l2, int l3) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(l1 + 1)) return result;
    
    result.reserve(prices.size());
    
    // Initialize with first l1 values
    for (int i = 0; i < l1; ++i) {
        result.push_back(prices[i]);
    }
    
    double fastest_sc = 2.0 / (l2 + 1.0);
    double slowest_sc = 2.0 / (l3 + 1.0);
    
    for (size_t i = l1; i < prices.size(); ++i) {
        // Calculate efficiency ratio
        double change = std::abs(prices[i] - prices[i - l1]);
        double volatility = 0.0;
        
        for (int j = 1; j <= l1; ++j) {
            volatility += std::abs(prices[i - j + 1] - prices[i - j]);
        }
        
        double er = volatility > 0 ? change / volatility : 0;
        double sc = std::pow(er * (fastest_sc - slowest_sc) + slowest_sc, 2);
        
        // KAMA calculation
        result.push_back(result.back() + sc * (prices[i] - result.back()));
    }
    return result;
}

std::vector<double> TechnicalIndicators::linear_slope(const std::vector<double>& prices, int window_size) {
    std::vector<double> result;
    if (prices.size() < static_cast<size_t>(window_size)) return result;
    
    result.reserve(prices.size() - window_size + 1);
    
    for (size_t i = window_size - 1; i < prices.size(); ++i) {
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
        
        for (int j = 0; j < window_size; ++j) {
            double x = j;
            double y = prices[i - window_size + 1 + j];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double slope = (window_size * sum_xy - sum_x * sum_y) / 
                      (window_size * sum_x2 - sum_x * sum_x);
        result.push_back(slope);
    }
    return result;
}

std::vector<double> TechnicalIndicators::close_to_close_volatility(const std::vector<double>& prices, int window_size) {
    auto returns = calculate_returns(prices);
    return calculate_rolling_volatility(returns, window_size);
}

std::vector<double> TechnicalIndicators::parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size) {
    std::vector<double> result;
    if (high.size() != low.size() || high.size() < static_cast<size_t>(window_size)) {
        return result;
    }
    
    result.reserve(high.size() - window_size + 1);
    
    for (size_t i = window_size - 1; i < high.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < window_size; ++j) {
            size_t idx = i - window_size + 1 + j;
            if (low[idx] > 0) {
                double ratio = high[idx] / low[idx];
                sum += std::log(ratio) * std::log(ratio);
            }
        }
        result.push_back(std::sqrt(sum / window_size) / (4.0 * std::log(2.0)));
    }
    return result;
}