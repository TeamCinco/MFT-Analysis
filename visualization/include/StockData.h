#pragma once
#include <string>

struct StockData {
    float datetime_index;
    float open, high, low, close, volume;
    std::string symbol;
    std::string data_frequency;
    float returns, sma, rsi, volatility, momentum;
    float spread, internal_bar_strength, skewness_30, kurtosis_30;
    float log_pct_change_5, auto_correlation_50_10, kama_10_2_30;
    float linear_slope_20, linear_slope_60, parkinson_volatility_20;
    float volume_sma_20, velocity, acceleration;
    float candle_way, candle_filling, candle_amplitude;
    
    // New advanced features from CSV
    float z_score_20, percentile_rank_50, coefficient_of_variation_30;
    float detrended_price_oscillator_20, hurst_exponent_100, garch_volatility_21;
    float shannon_entropy_volume_10, chande_momentum_oscillator_14, aroon_oscillator_25;
    float trix_15, vortex_indicator_14, supertrend_10_3;
    float ichimoku_senkou_span_A_9_26, ichimoku_senkou_span_B_26_52, fisher_transform_10;
    float volume_weighted_average_price_intraday, volume_profile_high_volume_node_intraday;
    float volume_profile_low_volume_node_intraday, on_balance_volume_sma_20;
    float klinger_oscillator_34_55, money_flow_index_14, vwap_deviation_stddev_30;
    float markov_regime_switching_garch_2_state, adx_rating_14;
    float chow_test_statistic_breakpoint_detection_50, market_regime_hmm_3_states_price_vol;
    float high_volatility_indicator_garch_threshold, return_x_volume_interaction_10;
    float volatility_x_rsi_interaction_14, price_to_kama_ratio_20_10_30;
    float polynomial_regression_price_degree_2_slope, conditional_value_at_risk_cvar_95_20;
    float drawdown_duration_from_peak_50, ulcer_index_14, sortino_ratio_30;
    
    std::string date_string;
};
