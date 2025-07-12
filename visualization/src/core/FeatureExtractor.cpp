#include "core/FeatureExtractor.h"
#include "../../feature_engineering/include/ohlcv_data.h"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace Visualization {

// FlexibleStockData implementation
double FlexibleStockData::getFeature(const std::string& name, double default_value) const {
    auto it = features.find(name);
    return (it != features.end()) ? it->second : default_value;
}

void FlexibleStockData::setFeature(const std::string& name, double value) {
    features[name] = value;
}

bool FlexibleStockData::hasFeature(const std::string& name) const {
    return features.find(name) != features.end();
}

std::vector<std::string> FlexibleStockData::getFeatureNames() const {
    std::vector<std::string> names;
    names.reserve(features.size());
    for (const auto& pair : features) {
        names.push_back(pair.first);
    }
    return names;
}

// FeatureExtractor static members
std::unordered_map<std::string, FeatureExtractor::FeatureExtractorFunc> FeatureExtractor::feature_extractors_;
bool FeatureExtractor::extractors_initialized_ = false;

void FeatureExtractor::ensureExtractorsInitialized() {
    if (!extractors_initialized_) {
        initializeExtractors();
        extractors_initialized_ = true;
    }
}

void FeatureExtractor::initializeExtractors() {
    // Basic features
    feature_extractors_["returns"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.returns); };
    feature_extractors_["sma"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.sma); };
    feature_extractors_["rsi"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.rsi); };
    feature_extractors_["volatility"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volatility); };
    feature_extractors_["momentum"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.momentum); };
    feature_extractors_["spread"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.spread); };
    feature_extractors_["internal_bar_strength"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.internal_bar_strength); };
    
    // Statistical features
    feature_extractors_["skewness_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.skewness_30); };
    feature_extractors_["kurtosis_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.kurtosis_30); };
    feature_extractors_["log_pct_change_5"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.log_pct_change_5); };
    feature_extractors_["auto_correlation_50_10"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.auto_correlation_50_10); };
    feature_extractors_["z_score_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.z_score_20); };
    feature_extractors_["percentile_rank_50"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.percentile_rank_50); };
    feature_extractors_["coefficient_of_variation_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.coefficient_of_variation_30); };
    feature_extractors_["detrended_price_oscillator_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.detrended_price_oscillator_20); };
    feature_extractors_["hurst_exponent_100"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.hurst_exponent_100); };
    feature_extractors_["garch_volatility_21"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.garch_volatility_21); };
    feature_extractors_["shannon_entropy_volume_10"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.shannon_entropy_volume_10); };
    
    // Advanced technical indicators
    feature_extractors_["kama_10_2_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.kama_10_2_30); };
    feature_extractors_["linear_slope_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.linear_slope_20); };
    feature_extractors_["linear_slope_60"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.linear_slope_60); };
    feature_extractors_["parkinson_volatility_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.parkinson_volatility_20); };
    feature_extractors_["volume_sma_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volume_sma_20); };
    feature_extractors_["velocity"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.velocity); };
    feature_extractors_["acceleration"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.acceleration); };
    
    // Candlestick features
    feature_extractors_["candle_way"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.candle_way); };
    feature_extractors_["candle_filling"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.candle_filling); };
    feature_extractors_["candle_amplitude"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.candle_amplitude); };
    
    // Extended technical analysis
    feature_extractors_["chande_momentum_oscillator_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.chande_momentum_oscillator_14); };
    feature_extractors_["aroon_oscillator_25"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.aroon_oscillator_25); };
    feature_extractors_["trix_15"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.trix_15); };
    feature_extractors_["vortex_indicator_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.vortex_indicator_14); };
    feature_extractors_["supertrend_10_3"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.supertrend_10_3); };
    feature_extractors_["ichimoku_senkou_span_A_9_26"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.ichimoku_senkou_span_A_9_26); };
    feature_extractors_["ichimoku_senkou_span_B_26_52"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.ichimoku_senkou_span_B_26_52); };
    feature_extractors_["fisher_transform_10"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.fisher_transform_10); };
    
    // Volume/Liquidity advanced
    feature_extractors_["volume_weighted_average_price_intraday"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volume_weighted_average_price_intraday); };
    feature_extractors_["volume_profile_high_volume_node_intraday"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volume_profile_high_volume_node_intraday); };
    feature_extractors_["volume_profile_low_volume_node_intraday"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volume_profile_low_volume_node_intraday); };
    feature_extractors_["on_balance_volume_sma_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.on_balance_volume_sma_20); };
    feature_extractors_["klinger_oscillator_34_55"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.klinger_oscillator_34_55); };
    feature_extractors_["money_flow_index_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.money_flow_index_14); };
    feature_extractors_["vwap_deviation_stddev_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.vwap_deviation_stddev_30); };
    
    // Regime detection
    feature_extractors_["markov_regime_switching_garch_2_state"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.markov_regime_switching_garch_2_state); };
    feature_extractors_["adx_rating_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.adx_rating_14); };
    feature_extractors_["chow_test_statistic_breakpoint_detection_50"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.chow_test_statistic_breakpoint_detection_50); };
    feature_extractors_["market_regime_hmm_3_states_price_vol"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.market_regime_hmm_3_states_price_vol); };
    feature_extractors_["high_volatility_indicator_garch_threshold"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.high_volatility_indicator_garch_threshold); };
    
    // Non-linear/Interaction
    feature_extractors_["return_x_volume_interaction_10"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.return_x_volume_interaction_10); };
    feature_extractors_["volatility_x_rsi_interaction_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.volatility_x_rsi_interaction_14); };
    feature_extractors_["price_to_kama_ratio_20_10_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.price_to_kama_ratio_20_10_30); };
    feature_extractors_["polynomial_regression_price_degree_2_slope"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.polynomial_regression_price_degree_2_slope); };
    
    // Alternative risk measures
    feature_extractors_["conditional_value_at_risk_cvar_95_20"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.conditional_value_at_risk_cvar_95_20); };
    feature_extractors_["drawdown_duration_from_peak_50"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.drawdown_duration_from_peak_50); };
    feature_extractors_["ulcer_index_14"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.ulcer_index_14); };
    feature_extractors_["sortino_ratio_30"] = [](const FeatureSet& fs) { return extractVectorFromMember(fs.sortino_ratio_30); };
}

std::vector<FlexibleStockData> FeatureExtractor::extractFromFeatureSet(
    const std::string& symbol,
    const OHLCVData& ohlcv_data,
    const FeatureSet& feature_set) {
    
    ensureExtractorsInitialized();
    
    std::vector<FlexibleStockData> result;
    
    // Determine the size based on the largest available data
    size_t data_size = 0;
    if (!ohlcv_data.close.empty()) {
        data_size = ohlcv_data.close.size();
    } else if (!feature_set.returns.empty()) {
        data_size = feature_set.returns.size();
    } else {
        return result; // No data available
    }
    
    result.reserve(data_size);
    
    // Convert timestamps to indices and strings
    auto datetime_indices = convertTimestampsToIndices(ohlcv_data.timestamps);
    auto date_strings = convertTimestampsToStrings(ohlcv_data.timestamps);
    
    // Create FlexibleStockData for each time point
    for (size_t i = 0; i < data_size; ++i) {
        FlexibleStockData data_point;
        data_point.symbol = symbol;
        
        // Set timestamp information
        if (i < ohlcv_data.timestamps.size()) {
            data_point.timestamp = ohlcv_data.timestamps[i];
            data_point.date_string = date_strings[i];
            data_point.datetime_index = datetime_indices[i];
        }
        
        // Add OHLCV data
        if (i < ohlcv_data.open.size()) data_point.setFeature("open", ohlcv_data.open[i]);
        if (i < ohlcv_data.high.size()) data_point.setFeature("high", ohlcv_data.high[i]);
        if (i < ohlcv_data.low.size()) data_point.setFeature("low", ohlcv_data.low[i]);
        if (i < ohlcv_data.close.size()) data_point.setFeature("close", ohlcv_data.close[i]);
        if (i < ohlcv_data.volume.size()) data_point.setFeature("volume", ohlcv_data.volume[i]);
        
        // Extract all available features
        for (const auto& extractor_pair : feature_extractors_) {
            const std::string& feature_name = extractor_pair.first;
            const auto& extractor_func = extractor_pair.second;
            
            try {
                auto feature_vector = extractor_func(feature_set);
                if (i < feature_vector.size() && !std::isnan(feature_vector[i])) {
                    data_point.setFeature(feature_name, feature_vector[i]);
                }
            } catch (...) {
                // Skip features that can't be extracted
                continue;
            }
        }
        
        result.push_back(std::move(data_point));
    }
    
    return result;
}

std::vector<double> FeatureExtractor::extractFeatureVector(
    const FeatureSet& feature_set, 
    const std::string& feature_name) {
    
    ensureExtractorsInitialized();
    
    auto it = feature_extractors_.find(feature_name);
    if (it != feature_extractors_.end()) {
        return it->second(feature_set);
    }
    
    return std::vector<double>(); // Return empty vector if feature not found
}

std::unordered_map<std::string, std::vector<double>> FeatureExtractor::extractMultipleFeatures(
    const FeatureSet& feature_set,
    const std::vector<std::string>& feature_names) {
    
    std::unordered_map<std::string, std::vector<double>> result;
    
    for (const auto& feature_name : feature_names) {
        auto feature_vector = extractFeatureVector(feature_set, feature_name);
        if (!feature_vector.empty()) {
            result[feature_name] = std::move(feature_vector);
        }
    }
    
    return result;
}

std::vector<std::string> FeatureExtractor::getAvailableFeatures(const FeatureSet& feature_set) {
    ensureExtractorsInitialized();
    
    std::vector<std::string> available_features;
    
    for (const auto& extractor_pair : feature_extractors_) {
        const std::string& feature_name = extractor_pair.first;
        if (validateFeature(feature_set, feature_name)) {
            available_features.push_back(feature_name);
        }
    }
    
    return available_features;
}

bool FeatureExtractor::validateFeature(const FeatureSet& feature_set, const std::string& feature_name) {
    ensureExtractorsInitialized();
    
    auto it = feature_extractors_.find(feature_name);
    if (it == feature_extractors_.end()) {
        return false;
    }
    
    try {
        auto feature_vector = it->second(feature_set);
        return !feature_vector.empty();
    } catch (...) {
        return false;
    }
}

std::vector<float> FeatureExtractor::convertTimestampsToIndices(
    const std::vector<std::chrono::system_clock::time_point>& timestamps) {
    
    std::vector<float> indices;
    indices.reserve(timestamps.size());
    
    for (size_t i = 0; i < timestamps.size(); ++i) {
        indices.push_back(static_cast<float>(i));
    }
    
    return indices;
}

std::vector<std::string> FeatureExtractor::convertTimestampsToStrings(
    const std::vector<std::chrono::system_clock::time_point>& timestamps) {
    
    std::vector<std::string> date_strings;
    date_strings.reserve(timestamps.size());
    
    for (const auto& timestamp : timestamps) {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d");
        date_strings.push_back(ss.str());
    }
    
    return date_strings;
}

} // namespace Visualization
