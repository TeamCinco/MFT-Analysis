#include "core/FeatureRegistry.h"
#include <algorithm>

namespace Visualization {

void FeatureMetadata::setDefaultColor() {
    switch (category) {
        case FeatureCategory::PRICE:
            color[0] = 0.2f; color[1] = 0.6f; color[2] = 1.0f; // Blue
            break;
        case FeatureCategory::TECHNICAL:
            color[0] = 1.0f; color[1] = 0.5f; color[2] = 0.0f; // Orange
            break;
        case FeatureCategory::STATISTICAL:
            color[0] = 0.8f; color[1] = 0.2f; color[2] = 0.8f; // Purple
            break;
        case FeatureCategory::VOLUME:
            color[0] = 0.0f; color[1] = 0.8f; color[2] = 0.4f; // Green
            break;
        case FeatureCategory::REGIME:
            color[0] = 0.9f; color[1] = 0.1f; color[2] = 0.1f; // Red
            break;
        case FeatureCategory::RISK:
            color[0] = 0.6f; color[1] = 0.0f; color[2] = 0.0f; // Dark Red
            break;
        case FeatureCategory::INTERACTION:
            color[0] = 0.5f; color[1] = 0.5f; color[2] = 0.5f; // Gray
            break;
        case FeatureCategory::CANDLESTICK:
            color[0] = 1.0f; color[1] = 0.8f; color[2] = 0.0f; // Yellow
            break;
        case FeatureCategory::MOMENTUM:
            color[0] = 0.0f; color[1] = 0.6f; color[2] = 0.8f; // Cyan
            break;
        case FeatureCategory::VOLATILITY:
            color[0] = 0.8f; color[1] = 0.4f; color[2] = 0.0f; // Brown
            break;
        default:
            color[0] = 0.5f; color[1] = 0.5f; color[2] = 0.5f; // Default gray
            break;
    }
}

FeatureRegistry& FeatureRegistry::getInstance() {
    static FeatureRegistry instance;
    return instance;
}

void FeatureRegistry::registerFeature(const FeatureMetadata& metadata) {
    features_[metadata.name] = metadata;
    updateCategoryMap(metadata.name, metadata.category);
}

void FeatureRegistry::registerFeature(const std::string& name, 
                                     const std::string& display_name,
                                     FeatureCategory category,
                                     ChartType chart_type,
                                     const std::string& units,
                                     bool is_percentage) {
    FeatureMetadata metadata(name, display_name, category, chart_type, units, is_percentage);
    registerFeature(metadata);
}

const FeatureMetadata* FeatureRegistry::getFeature(const std::string& name) const {
    auto it = features_.find(name);
    return (it != features_.end()) ? &it->second : nullptr;
}

std::vector<std::string> FeatureRegistry::getFeaturesByCategory(FeatureCategory category) const {
    auto it = category_map_.find(category);
    return (it != category_map_.end()) ? it->second : std::vector<std::string>();
}

std::vector<std::string> FeatureRegistry::getAllFeatureNames() const {
    std::vector<std::string> names;
    names.reserve(features_.size());
    for (const auto& pair : features_) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<FeatureCategory> FeatureRegistry::getAllCategories() const {
    std::vector<FeatureCategory> categories;
    for (const auto& pair : category_map_) {
        if (!pair.second.empty()) {
            categories.push_back(pair.first);
        }
    }
    return categories;
}

std::string FeatureRegistry::getCategoryName(FeatureCategory category) const {
    switch (category) {
        case FeatureCategory::PRICE: return "Price Data";
        case FeatureCategory::TECHNICAL: return "Technical Indicators";
        case FeatureCategory::STATISTICAL: return "Statistical Features";
        case FeatureCategory::VOLUME: return "Volume Analysis";
        case FeatureCategory::REGIME: return "Regime Detection";
        case FeatureCategory::RISK: return "Risk Measures";
        case FeatureCategory::INTERACTION: return "Interaction Features";
        case FeatureCategory::CANDLESTICK: return "Candlestick Patterns";
        case FeatureCategory::MOMENTUM: return "Momentum Indicators";
        case FeatureCategory::VOLATILITY: return "Volatility Measures";
        default: return "Unknown";
    }
}

bool FeatureRegistry::isFeatureRegistered(const std::string& name) const {
    return features_.find(name) != features_.end();
}

size_t FeatureRegistry::getFeatureCount() const {
    return features_.size();
}

void FeatureRegistry::updateCategoryMap(const std::string& name, FeatureCategory category) {
    category_map_[category].push_back(name);
}

void FeatureRegistry::initializeDefaultFeatures() {
    // Basic OHLCV features
    registerFeature("open", "Open Price", FeatureCategory::PRICE, ChartType::LINE, "$");
    registerFeature("high", "High Price", FeatureCategory::PRICE, ChartType::LINE, "$");
    registerFeature("low", "Low Price", FeatureCategory::PRICE, ChartType::LINE, "$");
    registerFeature("close", "Close Price", FeatureCategory::PRICE, ChartType::LINE, "$");
    registerFeature("volume", "Volume", FeatureCategory::VOLUME, ChartType::BAR, "shares");
    
    // Basic technical indicators
    registerFeature("returns", "Returns", FeatureCategory::TECHNICAL, ChartType::LINE, "", true);
    registerFeature("sma", "Simple Moving Average", FeatureCategory::TECHNICAL, ChartType::LINE, "$");
    registerFeature("rsi", "RSI", FeatureCategory::TECHNICAL, ChartType::LINE, "", false);
    registerFeature("volatility", "Volatility", FeatureCategory::VOLATILITY, ChartType::LINE, "", true);
    registerFeature("momentum", "Momentum", FeatureCategory::MOMENTUM, ChartType::LINE);
    registerFeature("spread", "Spread", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("internal_bar_strength", "Internal Bar Strength", FeatureCategory::TECHNICAL, ChartType::LINE);
    
    // Statistical features
    registerFeature("skewness_30", "Skewness (30)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("kurtosis_30", "Kurtosis (30)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("log_pct_change_5", "Log Pct Change (5)", FeatureCategory::STATISTICAL, ChartType::LINE, "", true);
    registerFeature("auto_correlation_50_10", "Auto Correlation (50,10)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("z_score_20", "Z-Score (20)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("percentile_rank_50", "Percentile Rank (50)", FeatureCategory::STATISTICAL, ChartType::LINE, "", true);
    registerFeature("coefficient_of_variation_30", "Coefficient of Variation (30)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("hurst_exponent_100", "Hurst Exponent (100)", FeatureCategory::STATISTICAL, ChartType::LINE);
    registerFeature("shannon_entropy_volume_10", "Shannon Entropy Volume (10)", FeatureCategory::STATISTICAL, ChartType::LINE);
    
    // Advanced technical indicators
    registerFeature("kama_10_2_30", "KAMA (10,2,30)", FeatureCategory::TECHNICAL, ChartType::LINE, "$");
    registerFeature("linear_slope_20", "Linear Slope (20)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("linear_slope_60", "Linear Slope (60)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("parkinson_volatility_20", "Parkinson Volatility (20)", FeatureCategory::VOLATILITY, ChartType::LINE, "", true);
    registerFeature("volume_sma_20", "Volume SMA (20)", FeatureCategory::VOLUME, ChartType::LINE, "shares");
    registerFeature("velocity", "Velocity", FeatureCategory::MOMENTUM, ChartType::LINE);
    registerFeature("acceleration", "Acceleration", FeatureCategory::MOMENTUM, ChartType::LINE);
    registerFeature("detrended_price_oscillator_20", "Detrended Price Oscillator (20)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("garch_volatility_21", "GARCH Volatility (21)", FeatureCategory::VOLATILITY, ChartType::LINE, "", true);
    
    // Extended technical analysis
    registerFeature("chande_momentum_oscillator_14", "Chande Momentum Oscillator (14)", FeatureCategory::MOMENTUM, ChartType::LINE);
    registerFeature("aroon_oscillator_25", "Aroon Oscillator (25)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("trix_15", "TRIX (15)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("vortex_indicator_14", "Vortex Indicator (14)", FeatureCategory::TECHNICAL, ChartType::LINE);
    registerFeature("supertrend_10_3", "SuperTrend (10,3)", FeatureCategory::TECHNICAL, ChartType::LINE, "$");
    registerFeature("ichimoku_senkou_span_A_9_26", "Ichimoku Senkou Span A (9,26)", FeatureCategory::TECHNICAL, ChartType::LINE, "$");
    registerFeature("ichimoku_senkou_span_B_26_52", "Ichimoku Senkou Span B (26,52)", FeatureCategory::TECHNICAL, ChartType::LINE, "$");
    registerFeature("fisher_transform_10", "Fisher Transform (10)", FeatureCategory::TECHNICAL, ChartType::LINE);
    
    // Volume/Liquidity advanced
    registerFeature("volume_weighted_average_price_intraday", "VWAP Intraday", FeatureCategory::VOLUME, ChartType::LINE, "$");
    registerFeature("volume_profile_high_volume_node_intraday", "Volume Profile High Node", FeatureCategory::VOLUME, ChartType::LINE, "$");
    registerFeature("volume_profile_low_volume_node_intraday", "Volume Profile Low Node", FeatureCategory::VOLUME, ChartType::LINE, "$");
    registerFeature("on_balance_volume_sma_20", "OBV SMA (20)", FeatureCategory::VOLUME, ChartType::LINE);
    registerFeature("klinger_oscillator_34_55", "Klinger Oscillator (34,55)", FeatureCategory::VOLUME, ChartType::LINE);
    registerFeature("money_flow_index_14", "Money Flow Index (14)", FeatureCategory::VOLUME, ChartType::LINE);
    registerFeature("vwap_deviation_stddev_30", "VWAP Deviation StdDev (30)", FeatureCategory::VOLUME, ChartType::LINE);
    
    // Regime detection
    registerFeature("markov_regime_switching_garch_2_state", "Markov Regime GARCH (2-state)", FeatureCategory::REGIME, ChartType::LINE);
    registerFeature("adx_rating_14", "ADX Rating (14)", FeatureCategory::REGIME, ChartType::LINE);
    registerFeature("chow_test_statistic_breakpoint_detection_50", "Chow Test Breakpoint (50)", FeatureCategory::REGIME, ChartType::LINE);
    registerFeature("market_regime_hmm_3_states_price_vol", "Market Regime HMM (3-states)", FeatureCategory::REGIME, ChartType::LINE);
    registerFeature("high_volatility_indicator_garch_threshold", "High Volatility GARCH Threshold", FeatureCategory::REGIME, ChartType::LINE);
    
    // Non-linear/Interaction
    registerFeature("return_x_volume_interaction_10", "Return x Volume Interaction (10)", FeatureCategory::INTERACTION, ChartType::LINE);
    registerFeature("volatility_x_rsi_interaction_14", "Volatility x RSI Interaction (14)", FeatureCategory::INTERACTION, ChartType::LINE);
    registerFeature("price_to_kama_ratio_20_10_30", "Price to KAMA Ratio (20,10,30)", FeatureCategory::INTERACTION, ChartType::LINE);
    registerFeature("polynomial_regression_price_degree_2_slope", "Polynomial Regression Price Slope (degree 2)", FeatureCategory::INTERACTION, ChartType::LINE);
    
    // Alternative risk measures
    registerFeature("conditional_value_at_risk_cvar_95_20", "CVaR 95% (20)", FeatureCategory::RISK, ChartType::LINE, "", true);
    registerFeature("drawdown_duration_from_peak_50", "Drawdown Duration from Peak (50)", FeatureCategory::RISK, ChartType::LINE, "days");
    registerFeature("ulcer_index_14", "Ulcer Index (14)", FeatureCategory::RISK, ChartType::LINE);
    registerFeature("sortino_ratio_30", "Sortino Ratio (30)", FeatureCategory::RISK, ChartType::LINE);
    
    // Candlestick patterns
    registerFeature("candle_way", "Candle Way", FeatureCategory::CANDLESTICK, ChartType::LINE);
    registerFeature("candle_filling", "Candle Filling", FeatureCategory::CANDLESTICK, ChartType::LINE, "", true);
    registerFeature("candle_amplitude", "Candle Amplitude", FeatureCategory::CANDLESTICK, ChartType::LINE);
    
    // Set bounds for percentage-based features
    auto rsi_feature = const_cast<FeatureMetadata*>(getFeature("rsi"));
    if (rsi_feature) rsi_feature->setBounds(0.0f, 100.0f);
}

void registerAllFeatures() {
    FeatureRegistry::getInstance().initializeDefaultFeatures();
}

} // namespace Visualization
