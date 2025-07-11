#include "batch_processor.h"
#include "technical_indicators.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>  

BatchOHLCProcessor::BatchOHLCProcessor() {
    workspace_.reserve(100000); // Larger workspace for better performance
}

void BatchOHLCProcessor::FeatureSet::resize_all(size_t size) {
    try {
        returns.resize(size, 0.0);
        sma.resize(size, 0.0);
        rsi.resize(size, 0.0);
        volatility.resize(size, 0.0);
        momentum.resize(size, 0.0);
        spread.resize(size, 0.0);
        internal_bar_strength.resize(size, 0.0);
        skewness_30.resize(size, 0.0);
        kurtosis_30.resize(size, 0.0);
        log_pct_change_5.resize(size, 0.0);
        auto_correlation_50_10.resize(size, 0.0);
        kama_10_2_30.resize(size, 0.0);
        linear_slope_20.resize(size, 0.0);
        linear_slope_60.resize(size, 0.0);
        parkinson_volatility_20.resize(size, 0.0);
        volume_sma_20.resize(size, 0.0);
        velocity.resize(size, 0.0);
        acceleration.resize(size, 0.0);
        candle_way.resize(size, 0);
        candle_filling.resize(size, 0.0);
        candle_amplitude.resize(size, 0.0);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to resize feature vectors: " + std::string(e.what()));
    }
}

void BatchOHLCProcessor::FeatureSet::clear_all() {
    returns.clear();
    sma.clear();
    rsi.clear();
    volatility.clear();
    momentum.clear();
    spread.clear();
    internal_bar_strength.clear();
    skewness_30.clear();
    kurtosis_30.clear();
    log_pct_change_5.clear();
    auto_correlation_50_10.clear();
    kama_10_2_30.clear();
    linear_slope_20.clear();
    linear_slope_60.clear();
    parkinson_volatility_20.clear();
    volume_sma_20.clear();
    velocity.clear();
    acceleration.clear();
    candle_way.clear();
    candle_filling.clear();
    candle_amplitude.clear();
}

BatchOHLCProcessor::FeatureSet BatchOHLCProcessor::calculate_organized_features(
    const std::vector<double>& open,
    const std::vector<double>& high,
    const std::vector<double>& low,
    const std::vector<double>& close,
    const std::vector<double>& volume
) {
    FeatureSet features;
    
    if (open.empty() || high.empty() || low.empty() || close.empty() || volume.empty()) {
        throw std::runtime_error("Input vectors cannot be empty");
    }
    
    const size_t data_size = close.size();
    
    // Validate input sizes
    if (open.size() != data_size || high.size() != data_size || 
        low.size() != data_size || volume.size() != data_size) {
        throw std::runtime_error("All input vectors must have the same size");
    }
    
    try {
        // Calculate basic price features
        features.returns = TechnicalIndicators::calculate_returns(close);
        features.sma = TechnicalIndicators::simple_moving_average(close, 20);
        features.rsi = TechnicalIndicators::calculate_rsi(close, 14);
        
        // Calculate volatility safely
        if (!features.returns.empty()) {
            features.volatility = TechnicalIndicators::calculate_rolling_volatility(features.returns, 20);
        }
        
        features.momentum = TechnicalIndicators::calculate_momentum(close, 10);
        
        // Calculate candle features
        features.spread = TechnicalIndicators::compute_spread(high, low);
        features.internal_bar_strength = TechnicalIndicators::internal_bar_strength(open, high, low, close);
        
        auto candle_info = TechnicalIndicators::candle_information(open, high, low, close);
        features.candle_way = std::move(candle_info.first);
        features.candle_filling = std::move(candle_info.second.first);
        features.candle_amplitude = std::move(candle_info.second.second);
        
        // Calculate math features with error handling
        try {
            features.skewness_30 = TechnicalIndicators::skewness(close, 30);
        } catch (...) {
            features.skewness_30.resize(data_size, 0.0);
        }
        
        try {
            features.kurtosis_30 = TechnicalIndicators::kurtosis(close, 30);
        } catch (...) {
            features.kurtosis_30.resize(data_size, 0.0);
        }
        
        try {
            features.log_pct_change_5 = TechnicalIndicators::log_pct_change(close, 5);
        } catch (...) {
            features.log_pct_change_5.resize(data_size, 0.0);
        }
        
        try {
            features.auto_correlation_50_10 = TechnicalIndicators::auto_correlation(close, 50, 10);
        } catch (...) {
            features.auto_correlation_50_10.resize(data_size, 0.0);
        }
        
        // Calculate trend features
        try {
            features.kama_10_2_30 = TechnicalIndicators::kama(close, 10, 2, 30);
        } catch (...) {
            features.kama_10_2_30.resize(data_size, 0.0);
        }
        
        try {
            features.linear_slope_20 = TechnicalIndicators::linear_slope(close, 20);
        } catch (...) {
            features.linear_slope_20.resize(data_size, 0.0);
        }
        
        try {
            features.linear_slope_60 = TechnicalIndicators::linear_slope(close, 60);
        } catch (...) {
            features.linear_slope_60.resize(data_size, 0.0);
        }
        
        // Calculate volatility features
        try {
            features.parkinson_volatility_20 = TechnicalIndicators::parkinson_volatility(high, low, 20);
        } catch (...) {
            features.parkinson_volatility_20.resize(data_size, 0.0);
        }
        
        // Calculate volume features
        features.volume_sma_20 = TechnicalIndicators::simple_moving_average(volume, 20);
        
        // Calculate derivatives
        try {
            auto derivatives_pair = TechnicalIndicators::derivatives(close);
            features.velocity = std::move(derivatives_pair.first);
            features.acceleration = std::move(derivatives_pair.second);
        } catch (...) {
            features.velocity.resize(data_size, 0.0);
            features.acceleration.resize(data_size, 0.0);
        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error calculating features: " + std::string(e.what()));
    }
    
    return features;
}

// Simplified versions of other methods
std::vector<std::vector<double>> BatchOHLCProcessor::batch_calculate_features(
    const std::vector<std::vector<double>>& close_prices,
    const std::vector<std::vector<double>>& volumes,
    int sma_period,
    int rsi_period
) {
    std::vector<std::vector<double>> features(close_prices.size());
    
    for (size_t idx = 0; idx < close_prices.size(); ++idx) {
        try {
            features[idx] = calculate_stock_features(
                close_prices[idx], volumes[idx], sma_period, rsi_period
            );
        } catch (const std::exception& e) {
            std::cerr << "Error processing batch item " << idx << ": " << e.what() << std::endl;
            features[idx] = std::vector<double>(); // Empty vector on error
        }
    }
    
    return features;
}

std::vector<double> BatchOHLCProcessor::calculate_stock_features(
    const std::vector<double>& close_prices,
    const std::vector<double>& volumes,
    int sma_period,
    int rsi_period
) {
    std::vector<double> features;
    
    if (close_prices.empty() || volumes.empty()) {
        return features;
    }
    
    try {
        // Calculate features safely
        auto returns = TechnicalIndicators::calculate_returns(close_prices);
        auto sma = TechnicalIndicators::simple_moving_average(close_prices, sma_period);
        auto volume_sma = TechnicalIndicators::simple_moving_average(volumes, sma_period);
        auto rsi = TechnicalIndicators::calculate_rsi(close_prices, rsi_period);
        auto volatility = TechnicalIndicators::calculate_rolling_volatility(returns, 20);
        auto momentum = TechnicalIndicators::calculate_momentum(close_prices, 10);
        
        // Reserve space
        size_t total_size = returns.size() + sma.size() + rsi.size() + 
                           volatility.size() + momentum.size() + volume_sma.size();
        features.reserve(total_size);
        
        // Combine features
        features.insert(features.end(), returns.begin(), returns.end());
        features.insert(features.end(), sma.begin(), sma.end());
        features.insert(features.end(), rsi.begin(), rsi.end());
        features.insert(features.end(), volatility.begin(), volatility.end());
        features.insert(features.end(), momentum.begin(), momentum.end());
        features.insert(features.end(), volume_sma.begin(), volume_sma.end());
        
    } catch (const std::exception& e) {
        std::cerr << "Error in calculate_stock_features: " << e.what() << std::endl;
        features.clear();
    }
    
    return features;
}

std::vector<double> BatchOHLCProcessor::calculate_comprehensive_features(
    const std::vector<double>& open,
    const std::vector<double>& high,
    const std::vector<double>& low,
    const std::vector<double>& close,
    const std::vector<double>& volume
) {
    std::vector<double> features;
    
    if (open.empty() || high.empty() || low.empty() || close.empty() || volume.empty()) {
        return features;
    }
    
    try {
        // Just use the basic features for now to avoid memory issues
        auto returns = TechnicalIndicators::calculate_returns(close);
        auto sma_20 = TechnicalIndicators::simple_moving_average(close, 20);
        auto rsi = TechnicalIndicators::calculate_rsi(close, 14);
        auto momentum = TechnicalIndicators::calculate_momentum(close, 10);
        auto spread = TechnicalIndicators::compute_spread(high, low);
        auto volume_sma = TechnicalIndicators::simple_moving_average(volume, 20);
        
        // Combine features
        features.reserve(returns.size() + sma_20.size() + rsi.size() + 
                        momentum.size() + spread.size() + volume_sma.size());
        
        features.insert(features.end(), returns.begin(), returns.end());
        features.insert(features.end(), sma_20.begin(), sma_20.end());
        features.insert(features.end(), rsi.begin(), rsi.end());
        features.insert(features.end(), momentum.begin(), momentum.end());
        features.insert(features.end(), spread.begin(), spread.end());
        features.insert(features.end(), volume_sma.begin(), volume_sma.end());
        
    } catch (const std::exception& e) {
        std::cerr << "Error in calculate_comprehensive_features: " << e.what() << std::endl;
        features.clear();
    }
    
    return features;
}

std::vector<std::vector<double>> BatchOHLCProcessor::batch_calculate_comprehensive_features(
    const std::vector<std::vector<double>>& open_prices,
    const std::vector<std::vector<double>>& high_prices,
    const std::vector<std::vector<double>>& low_prices,
    const std::vector<std::vector<double>>& close_prices,
    const std::vector<std::vector<double>>& volumes
) {
    std::vector<std::vector<double>> features(close_prices.size());
    
    for (size_t idx = 0; idx < close_prices.size(); ++idx) {
        try {
            features[idx] = calculate_comprehensive_features(
                open_prices[idx], high_prices[idx], low_prices[idx], 
                close_prices[idx], volumes[idx]
            );
        } catch (const std::exception& e) {
            std::cerr << "Error processing comprehensive batch item " << idx << ": " << e.what() << std::endl;
            features[idx] = std::vector<double>(); // Empty vector on error
        }
    }
    
    return features;
}