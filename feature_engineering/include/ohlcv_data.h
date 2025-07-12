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

    // Statistical/Mathematical Features
    std::vector<double> z_score_20;
    std::vector<double> percentile_rank_50;
    std::vector<double> coefficient_of_variation_30;
    std::vector<double> detrended_price_oscillator_20;
    std::vector<double> hurst_exponent_100;
    std::vector<double> garch_volatility_21;
    std::vector<double> shannon_entropy_volume_10;

    // Technical Analysis Extended
    std::vector<double> chande_momentum_oscillator_14;
    std::vector<double> aroon_oscillator_25;
    std::vector<double> trix_15;
    std::vector<double> vortex_indicator_14;
    std::vector<double> supertrend_10_3;
    std::vector<double> ichimoku_senkou_span_A_9_26;
    std::vector<double> ichimoku_senkou_span_B_26_52;
    std::vector<double> fisher_transform_10;

    // Volume/Liquidity Advanced
    std::vector<double> volume_weighted_average_price_intraday;
    std::vector<double> volume_profile_high_volume_node_intraday;
    std::vector<double> volume_profile_low_volume_node_intraday;
    std::vector<double> on_balance_volume_sma_20;
    std::vector<double> klinger_oscillator_34_55;
    std::vector<double> money_flow_index_14;
    std::vector<double> vwap_deviation_stddev_30;

    // Regime Detection
    std::vector<double> markov_regime_switching_garch_2_state;
    std::vector<double> adx_rating_14;
    std::vector<double> chow_test_statistic_breakpoint_detection_50;
    std::vector<double> market_regime_hmm_3_states_price_vol;
    std::vector<double> high_volatility_indicator_garch_threshold;

    // Non-Linear/Interaction
    std::vector<double> return_x_volume_interaction_10;
    std::vector<double> volatility_x_rsi_interaction_14;
    std::vector<double> price_to_kama_ratio_20_10_30;
    std::vector<double> polynomial_regression_price_degree_2_slope;

    // Alternative Risk Measures
    std::vector<double> conditional_value_at_risk_cvar_95_20;
    std::vector<double> drawdown_duration_from_peak_50;
    std::vector<double> ulcer_index_14;
    std::vector<double> sortino_ratio_30;
};
