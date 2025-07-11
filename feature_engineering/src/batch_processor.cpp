#include "batch_processor.h"
#include "technical_indicators.h"
#include <algorithm>

BatchOHLCProcessor::BatchOHLCProcessor() {
    workspace_.reserve(100000); // Larger workspace for better performance
}

void BatchOHLCProcessor::FeatureSet::resize_all(size_t size) {
    returns.resize(size);
    sma.resize(size);
    rsi.resize(size);
    volatility.resize(size);
    momentum.resize(size);
    spread.resize(size);
    internal_bar_strength.resize(size);
    skewness_30.resize(size);
    kurtosis_30.resize(size);
    log_pct_change_5.resize(size);
    auto_correlation_50_10.resize(size);
    kama_10_2_30.resize(size);
    linear_slope_20.resize(size);
    linear_slope_60.resize(size);
    parkinson_volatility_20.resize(size);
    volume_sma_20.resize(size);
    velocity.resize(size);
    acceleration.resize(size);
    candle_way.resize(size);
    candle_filling.resize(size);
    candle_amplitude.resize(size);
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
    const size_t data_size = close.size();
    
    // Pre-allocate all vectors for better memory performance
    features.returns.reserve(data_size);
    features.sma.reserve(data_size);
    features.rsi.reserve(data_size);
    features.volatility.reserve(data_size);
    features.momentum.reserve(data_size);
    features.spread.reserve(data_size);
    features.internal_bar_strength.reserve(data_size);
    features.skewness_30.reserve(data_size);
    features.kurtosis_30.reserve(data_size);
    features.log_pct_change_5.reserve(data_size);
    features.auto_correlation_50_10.reserve(data_size);
    features.kama_10_2_30.reserve(data_size);
    features.linear_slope_20.reserve(data_size);
    features.linear_slope_60.reserve(data_size);
    features.parkinson_volatility_20.reserve(data_size);
    features.volume_sma_20.reserve(data_size);
    features.velocity.reserve(data_size);
    features.acceleration.reserve(data_size);
    features.candle_way.reserve(data_size);
    features.candle_filling.reserve(data_size);
    features.candle_amplitude.reserve(data_size);
    
    // Calculate basic price features
    features.returns = TechnicalIndicators::calculate_returns(close);
    features.sma = TechnicalIndicators::simple_moving_average(close, 20);
    features.rsi = TechnicalIndicators::calculate_rsi(close, 14);
    features.volatility = TechnicalIndicators::calculate_rolling_volatility(features.returns, 20);
    features.momentum = TechnicalIndicators::calculate_momentum(close, 10);
    
    // Calculate candle features
    features.spread = TechnicalIndicators::compute_spread(high, low);
    features.internal_bar_strength = TechnicalIndicators::internal_bar_strength(open, high, low, close);
    
    auto candle_info = TechnicalIndicators::candle_information(open, high, low, close);
    features.candle_way = std::move(candle_info.first);
    features.candle_filling = std::move(candle_info.second.first);
    features.candle_amplitude = std::move(candle_info.second.second);
    
    // Calculate math features
    features.skewness_30 = TechnicalIndicators::skewness(close, 30);
    features.kurtosis_30 = TechnicalIndicators::kurtosis(close, 30);
    features.log_pct_change_5 = TechnicalIndicators::log_pct_change(close, 5);
    features.auto_correlation_50_10 = TechnicalIndicators::auto_correlation(close, 50, 10);
    
    // Calculate trend features
    features.kama_10_2_30 = TechnicalIndicators::kama(close, 10, 2, 30);
    features.linear_slope_20 = TechnicalIndicators::linear_slope(close, 20);
    features.linear_slope_60 = TechnicalIndicators::linear_slope(close, 60);
    
    // Calculate volatility features
    features.parkinson_volatility_20 = TechnicalIndicators::parkinson_volatility(high, low, 20);
    
    // Calculate volume features
    features.volume_sma_20 = TechnicalIndicators::simple_moving_average(volume, 20);
    
    // Calculate derivatives
    auto derivatives_pair = TechnicalIndicators::derivatives(close);
    features.velocity = std::move(derivatives_pair.first);
    features.acceleration = std::move(derivatives_pair.second);
    
    return features;
}

std::vector<std::vector<double>> BatchOHLCProcessor::batch_calculate_features(
    const std::vector<std::vector<double>>& close_prices,
    const std::vector<std::vector<double>>& volumes,
    int sma_period,
    int rsi_period
) {
    std::vector<std::vector<double>> features(close_prices.size());
    
    // Process in parallel if beneficial
    for (size_t idx = 0; idx < close_prices.size(); ++idx) {
        features[idx] = calculate_stock_features(
            close_prices[idx], volumes[idx], sma_period, rsi_period
        );
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
        features[idx] = calculate_comprehensive_features(
            open_prices[idx], high_prices[idx], low_prices[idx], 
            close_prices[idx], volumes[idx]
        );
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
    
    // Calculate features
    auto returns = TechnicalIndicators::calculate_returns(close_prices);
    auto sma = TechnicalIndicators::simple_moving_average(close_prices, sma_period);
    auto volume_sma = TechnicalIndicators::simple_moving_average(volumes, sma_period);
    auto rsi = TechnicalIndicators::calculate_rsi(close_prices, rsi_period);
    auto volatility = TechnicalIndicators::calculate_rolling_volatility(returns, 20);
    auto momentum = TechnicalIndicators::calculate_momentum(close_prices, 10);
    
    // Pre-allocate for better performance
    size_t total_size = returns.size() + sma.size() + rsi.size() + 
                       volatility.size() + momentum.size() + volume_sma.size();
    features.reserve(total_size);
    
    // Combine features using move semantics for better performance
    features.insert(features.end(), std::make_move_iterator(returns.begin()), std::make_move_iterator(returns.end()));
    features.insert(features.end(), std::make_move_iterator(sma.begin()), std::make_move_iterator(sma.end()));
    features.insert(features.end(), std::make_move_iterator(rsi.begin()), std::make_move_iterator(rsi.end()));
    features.insert(features.end(), std::make_move_iterator(volatility.begin()), std::make_move_iterator(volatility.end()));
    features.insert(features.end(), std::make_move_iterator(momentum.begin()), std::make_move_iterator(momentum.end()));
    features.insert(features.end(), std::make_move_iterator(volume_sma.begin()), std::make_move_iterator(volume_sma.end()));
    
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
    
    // Calculate all features
    auto returns = TechnicalIndicators::calculate_returns(close);
    auto sma_20 = TechnicalIndicators::simple_moving_average(close, 20);
    auto sma_50 = TechnicalIndicators::simple_moving_average(close, 50);
    auto rsi = TechnicalIndicators::calculate_rsi(close, 14);
    auto momentum = TechnicalIndicators::calculate_momentum(close, 10);
    
    auto spread = TechnicalIndicators::compute_spread(high, low);
    auto ibs = TechnicalIndicators::internal_bar_strength(open, high, low, close);
    auto candle_info = TechnicalIndicators::candle_information(open, high, low, close);
    auto candle_way = std::move(candle_info.first);
    auto filling = std::move(candle_info.second.first);
    auto amplitude = std::move(candle_info.second.second);
    
    auto derivatives_pair = TechnicalIndicators::derivatives(close);
    auto velocity = std::move(derivatives_pair.first);
    auto acceleration = std::move(derivatives_pair.second);
    auto log_pct_5 = TechnicalIndicators::log_pct_change(close, 5);
    auto log_pct_20 = TechnicalIndicators::log_pct_change(close, 20);
    auto auto_corr = TechnicalIndicators::auto_correlation(close, 50, 10);
    auto skew_30 = TechnicalIndicators::skewness(close, 30);
    auto kurt_30 = TechnicalIndicators::kurtosis(close, 30);
    
    auto kama_10 = TechnicalIndicators::kama(close, 10, 2, 30);
    auto linear_slope_20 = TechnicalIndicators::linear_slope(close, 20);
    auto linear_slope_60 = TechnicalIndicators::linear_slope(close, 60);
    
    auto ctc_vol_20 = TechnicalIndicators::close_to_close_volatility(close, 20);
    auto parkinson_vol = TechnicalIndicators::parkinson_volatility(high, low, 20);
    
    auto volume_sma = TechnicalIndicators::simple_moving_average(volume, 20);
    
    // Calculate total size for pre-allocation
    size_t total_size = returns.size() + sma_20.size() + sma_50.size() + rsi.size() + 
                       momentum.size() + spread.size() + ibs.size() + 
                       filling.size() + amplitude.size() + velocity.size() + 
                       acceleration.size() + log_pct_5.size() + log_pct_20.size() + 
                       auto_corr.size() + skew_30.size() + kurt_30.size() + 
                       kama_10.size() + linear_slope_20.size() + linear_slope_60.size() + 
                       ctc_vol_20.size() + parkinson_vol.size() + volume_sma.size() +
                       candle_way.size();
    
    features.reserve(total_size);
    
    // Combine all features using move semantics
    features.insert(features.end(), std::make_move_iterator(returns.begin()), std::make_move_iterator(returns.end()));
    features.insert(features.end(), std::make_move_iterator(sma_20.begin()), std::make_move_iterator(sma_20.end()));
    features.insert(features.end(), std::make_move_iterator(sma_50.begin()), std::make_move_iterator(sma_50.end()));
    features.insert(features.end(), std::make_move_iterator(rsi.begin()), std::make_move_iterator(rsi.end()));
    features.insert(features.end(), std::make_move_iterator(momentum.begin()), std::make_move_iterator(momentum.end()));
    
    features.insert(features.end(), std::make_move_iterator(spread.begin()), std::make_move_iterator(spread.end()));
    features.insert(features.end(), std::make_move_iterator(ibs.begin()), std::make_move_iterator(ibs.end()));
    features.insert(features.end(), std::make_move_iterator(filling.begin()), std::make_move_iterator(filling.end()));
    features.insert(features.end(), std::make_move_iterator(amplitude.begin()), std::make_move_iterator(amplitude.end()));
    
    // Convert candle_way (int vector) to double
    for (int way : candle_way) {
        features.push_back(static_cast<double>(way));
    }
    
    features.insert(features.end(), std::make_move_iterator(velocity.begin()), std::make_move_iterator(velocity.end()));
    features.insert(features.end(), std::make_move_iterator(acceleration.begin()), std::make_move_iterator(acceleration.end()));
    features.insert(features.end(), std::make_move_iterator(log_pct_5.begin()), std::make_move_iterator(log_pct_5.end()));
    features.insert(features.end(), std::make_move_iterator(log_pct_20.begin()), std::make_move_iterator(log_pct_20.end()));
    features.insert(features.end(), std::make_move_iterator(auto_corr.begin()), std::make_move_iterator(auto_corr.end()));
    features.insert(features.end(), std::make_move_iterator(skew_30.begin()), std::make_move_iterator(skew_30.end()));
    features.insert(features.end(), std::make_move_iterator(kurt_30.begin()), std::make_move_iterator(kurt_30.end()));
    
    features.insert(features.end(), std::make_move_iterator(kama_10.begin()), std::make_move_iterator(kama_10.end()));
    features.insert(features.end(), std::make_move_iterator(linear_slope_20.begin()), std::make_move_iterator(linear_slope_20.end()));
    features.insert(features.end(), std::make_move_iterator(linear_slope_60.begin()), std::make_move_iterator(linear_slope_60.end()));
    
    features.insert(features.end(), std::make_move_iterator(ctc_vol_20.begin()), std::make_move_iterator(ctc_vol_20.end()));
    features.insert(features.end(), std::make_move_iterator(parkinson_vol.begin()), std::make_move_iterator(parkinson_vol.end()));
    
    features.insert(features.end(), std::make_move_iterator(volume_sma.begin()), std::make_move_iterator(volume_sma.end()));
    
    return features;
}