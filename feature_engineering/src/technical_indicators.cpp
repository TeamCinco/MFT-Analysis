#include "technical_indicators.h"
#include "simd_utils.h"
#include <cmath>
#include <algorithm>
#include <numeric>

// --- CANDLE INFORMATION ---
std::pair<std::vector<int>, std::pair<std::vector<double>, std::vector<double>>> 
TechnicalIndicators::candle_information(const std::vector<double>& open, const std::vector<double>& high,
                                       const std::vector<double>& low, const std::vector<double>& close) {
    std::vector<int> candle_way;
    std::vector<double> filling, amplitude;
    
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

std::vector<double> TechnicalIndicators::compute_spread(const std::vector<double>& high, const std::vector<double>& low) {
    std::vector<double> spread;
    for (size_t i = 0; i < high.size(); ++i) {
        spread.push_back(high[i] - low[i]);
    }
    return spread;
}

std::vector<double> TechnicalIndicators::internal_bar_strength(const std::vector<double>& open, const std::vector<double>& high,
                                                              const std::vector<double>& low, const std::vector<double>& close) {
    std::vector<double> ibs;
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

// --- MATH FEATURES ---
std::pair<std::vector<double>, std::vector<double>> TechnicalIndicators::derivatives(const std::vector<double>& prices) {
    std::vector<double> velocity, acceleration;
    
    if (prices.size() < 2) return std::make_pair(velocity, acceleration);
    
    // Velocity (first derivative)
    for (size_t i = 1; i < prices.size(); ++i) {
        velocity.push_back(prices[i] - prices[i-1]);
    }
    
    // Acceleration (second derivative)
    if (velocity.size() < 2) return std::make_pair(velocity, acceleration);
    
    for (size_t i = 1; i < velocity.size(); ++i) {
        acceleration.push_back(velocity[i] - velocity[i-1]);
    }
    
    return std::make_pair(velocity, acceleration);
}

std::vector<double> TechnicalIndicators::log_pct_change(const std::vector<double>& prices, int window_size) {
    std::vector<double> log_pct;
    
    for (size_t i = window_size; i < prices.size(); ++i) {
        if (prices[i - window_size] > 0 && prices[i] > 0) {
            log_pct.push_back(std::log(prices[i] / prices[i - window_size]));
        } else {
            log_pct.push_back(0.0);
        }
    }
    
    return log_pct;
}

std::vector<double> TechnicalIndicators::auto_correlation(const std::vector<double>& prices, int window_size, int lag) {
    std::vector<double> auto_corr;
    
    for (size_t i = window_size; i <= prices.size(); ++i) {
        if (i < static_cast<size_t>(lag)) {
            auto_corr.push_back(0.0);
            continue;
        }
        
        // Calculate autocorrelation for this window
        std::vector<double> window_data(prices.begin() + i - window_size, prices.begin() + i);
        std::vector<double> lagged_data(prices.begin() + i - window_size - lag, prices.begin() + i - lag);
        
        if (lagged_data.size() != window_data.size()) {
            auto_corr.push_back(0.0);
            continue;
        }
        
        // Calculate Pearson correlation
        double mean_x = std::accumulate(window_data.begin(), window_data.end(), 0.0) / window_data.size();
        double mean_y = std::accumulate(lagged_data.begin(), lagged_data.end(), 0.0) / lagged_data.size();
        
        double numerator = 0.0, sum_sq_x = 0.0, sum_sq_y = 0.0;
        
        for (size_t j = 0; j < window_data.size(); ++j) {
            double diff_x = window_data[j] - mean_x;
            double diff_y = lagged_data[j] - mean_y;
            numerator += diff_x * diff_y;
            sum_sq_x += diff_x * diff_x;
            sum_sq_y += diff_y * diff_y;
        }
        
        double denominator = std::sqrt(sum_sq_x * sum_sq_y);
        auto_corr.push_back(denominator > 0 ? numerator / denominator : 0.0);
    }
    
    return auto_corr;
}

std::vector<double> TechnicalIndicators::skewness(const std::vector<double>& prices, int window_size) {
    std::vector<double> skew_values;
    
    for (size_t i = window_size; i <= prices.size(); ++i) {
        std::vector<double> window_data(prices.begin() + i - window_size, prices.begin() + i);
        
        // Calculate mean
        double mean = std::accumulate(window_data.begin(), window_data.end(), 0.0) / window_size;
        
        // Calculate standard deviation and skewness
        double sum_sq_diff = 0.0, sum_cube_diff = 0.0;
        
        for (double value : window_data) {
            double diff = value - mean;
            sum_sq_diff += diff * diff;
            sum_cube_diff += diff * diff * diff;
        }
        
        double variance = sum_sq_diff / window_size;
        double std_dev = std::sqrt(variance);
        
        if (std_dev > 0) {
            double skewness_val = (sum_cube_diff / window_size) / (std_dev * std_dev * std_dev);
            skew_values.push_back(skewness_val);
        } else {
            skew_values.push_back(0.0);
        }
    }
    
    return skew_values;
}

std::vector<double> TechnicalIndicators::kurtosis(const std::vector<double>& prices, int window_size) {
    std::vector<double> kurt_values;
    
    for (size_t i = window_size; i <= prices.size(); ++i) {
        std::vector<double> window_data(prices.begin() + i - window_size, prices.begin() + i);
        
        // Calculate mean
        double mean = std::accumulate(window_data.begin(), window_data.end(), 0.0) / window_size;
        
        // Calculate moments
        double sum_sq_diff = 0.0, sum_fourth_diff = 0.0;
        
        for (double value : window_data) {
            double diff = value - mean;
            double sq_diff = diff * diff;
            sum_sq_diff += sq_diff;
            sum_fourth_diff += sq_diff * sq_diff;
        }
        
        double variance = sum_sq_diff / window_size;
        
        if (variance > 0) {
            double kurtosis_val = (sum_fourth_diff / window_size) / (variance * variance) - 3.0;  // Excess kurtosis
            kurt_values.push_back(kurtosis_val);
        } else {
            kurt_values.push_back(0.0);
        }
    }
    
    return kurt_values;
}

// --- TREND INDICATORS ---
std::vector<double> TechnicalIndicators::kama(const std::vector<double>& prices, int l1, int l2, int l3) {
    std::vector<double> kama_values;
    if (prices.size() < static_cast<size_t>(l1)) return kama_values;
    
    kama_values.push_back(prices[0]);  // First value
    
    for (size_t i = l1; i < prices.size(); ++i) {
        // Calculate Efficiency Ratio
        double change = std::abs(prices[i] - prices[i - l1]);
        
        double volatility = 0.0;
        for (size_t j = i - l1 + 1; j <= i; ++j) {
            volatility += std::abs(prices[j] - prices[j-1]);
        }
        
        double er = volatility > 0 ? change / volatility : 0.0;
        
        // Calculate Smoothing Constant
        double fastest_sc = 2.0 / (l2 + 1);
        double slowest_sc = 2.0 / (l3 + 1);
        double sc = er * (fastest_sc - slowest_sc) + slowest_sc;
        sc = sc * sc;  // Square for better smoothing
        
        // Calculate KAMA
        double kama_val = kama_values.back() + sc * (prices[i] - kama_values.back());
        kama_values.push_back(kama_val);
    }
    
    return kama_values;
}

std::vector<double> TechnicalIndicators::linear_slope(const std::vector<double>& prices, int window_size) {
    std::vector<double> slopes;
    
    for (size_t i = window_size; i <= prices.size(); ++i) {
        // Linear regression slope calculation
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        
        for (int j = 0; j < window_size; ++j) {
            double x = static_cast<double>(j);
            double y = prices[i - window_size + j];
            
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double n = static_cast<double>(window_size);
        double denominator = n * sum_x2 - sum_x * sum_x;
        
        if (denominator != 0) {
            double slope = (n * sum_xy - sum_x * sum_y) / denominator;
            slopes.push_back(slope);
        } else {
            slopes.push_back(0.0);
        }
    }
    
    return slopes;
}

// --- VOLATILITY INDICATORS ---
std::vector<double> TechnicalIndicators::close_to_close_volatility(const std::vector<double>& prices, int window_size) {
    auto returns = calculate_returns(prices);
    return calculate_rolling_volatility(returns, window_size);
}

std::vector<double> TechnicalIndicators::parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size) {
    std::vector<double> park_vol;
    
    for (size_t i = window_size; i <= high.size(); ++i) {
        double sum_log_hl = 0.0;
        
        for (size_t j = i - window_size; j < i; ++j) {
            if (low[j] > 0 && high[j] > 0) {
                double log_hl = std::log(high[j] / low[j]);
                sum_log_hl += log_hl * log_hl;
            }
        }
        
        double parkinson_est = std::sqrt(sum_log_hl / (4.0 * std::log(2.0) * window_size));
        park_vol.push_back(parkinson_est);
    }
    
    return park_vol;
}

// Add these missing implementations at the end of the file

std::vector<double> TechnicalIndicators::calculate_returns(const std::vector<double>& prices) {
    std::vector<double> returns;
    if (prices.size() < 2) return returns;
    
    returns.reserve(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        returns.push_back((prices[i] - prices[i-1]) / prices[i-1]);
    }
    return returns;
}

std::vector<double> TechnicalIndicators::calculate_rsi(const std::vector<double>& prices, int period) {
    if (prices.size() < static_cast<size_t>(period + 1)) return {};
    
    std::vector<double> gains, losses;
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }
    
    auto avg_gains = SIMDUtils::simd_rolling_mean(gains, period);
    auto avg_losses = SIMDUtils::simd_rolling_mean(losses, period);
    
    std::vector<double> rsi;
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

std::vector<double> TechnicalIndicators::simple_moving_average(const std::vector<double>& data, size_t window) {
    return SIMDUtils::simd_rolling_mean(data, window);
}

std::vector<double> TechnicalIndicators::calculate_rolling_volatility(const std::vector<double>& returns, int window) {
    std::vector<double> volatility;
    if (returns.size() < static_cast<size_t>(window)) return volatility;
    
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
    for (size_t i = static_cast<size_t>(period); i < prices.size(); ++i) {
        momentum.push_back((prices[i] - prices[i - period]) / prices[i - period]);
    }
    return momentum;
}