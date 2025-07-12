#pragma once
#include <vector>
#include <utility>

class TechnicalIndicators {
public:
    static std::vector<double> calculate_returns(const std::vector<double>& prices);
    static std::vector<double> simple_moving_average(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_rsi(const std::vector<double>& prices, int period);
    static std::vector<double> calculate_rolling_volatility(const std::vector<double>& returns, int window);
    static std::vector<double> compute_spread(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> internal_bar_strength(const std::vector<double>& open, const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close);
    static std::pair<std::vector<int>, std::pair<std::vector<double>, std::vector<double>>> candle_information(const std::vector<double>& open, const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close);
    static std::pair<std::vector<double>, std::vector<double>> derivatives(const std::vector<double>& prices);
    static std::vector<double> log_pct_change(const std::vector<double>& prices, int window_size);
    static std::vector<double> auto_correlation(const std::vector<double>& prices, int window_size, int lag);
    static std::vector<double> skewness(const std::vector<double>& prices, int window_size);
    static std::vector<double> kurtosis(const std::vector<double>& prices, int window_size);
    static std::vector<double> kama(const std::vector<double>& prices, int l1, int l2, int l3);
    static std::vector<double> linear_slope(const std::vector<double>& prices, int window_size);
    static std::vector<double> parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size);

    // Statistical/Mathematical
    static std::vector<double> z_score_20(const std::vector<double>& returns);
    static std::vector<double> percentile_rank_50(const std::vector<double>& prices);
    static std::vector<double> coefficient_of_variation_30(const std::vector<double>& returns);
    static std::vector<double> detrended_price_oscillator_20(const std::vector<double>& prices);
    static std::vector<double> hurst_exponent_100(const std::vector<double>& prices);
    static std::vector<double> garch_volatility_21(const std::vector<double>& returns);
    static std::vector<double> shannon_entropy_volume_10(const std::vector<double>& volume);

    // Technical Analysis Extended
    static std::vector<double> chande_momentum_oscillator_14(const std::vector<double>& prices);
    static std::vector<double> aroon_oscillator_25(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> trix_15(const std::vector<double>& prices);
    static std::vector<double> vortex_indicator_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close);
    static std::vector<double> supertrend_10_3(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close);
    static std::vector<double> ichimoku_senkou_span_A_9_26(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> ichimoku_senkou_span_B_26_52(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> fisher_transform_10(const std::vector<double>& high, const std::vector<double>& low);

    // Volume/Liquidity Advanced
    static std::vector<double> volume_weighted_average_price_intraday(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume);
    static std::vector<double> volume_profile_high_volume_node_intraday(const std::vector<double>& prices, const std::vector<double>& volume);
    static std::vector<double> volume_profile_low_volume_node_intraday(const std::vector<double>& prices, const std::vector<double>& volume);
    static std::vector<double> on_balance_volume_sma_20(const std::vector<double>& prices, const std::vector<double>& volume);
    static std::vector<double> klinger_oscillator_34_55(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume);
    static std::vector<double> money_flow_index_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume);
    static std::vector<double> vwap_deviation_stddev_30(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume);

    // Cross-Sectional/Relative
    static std::vector<double> relative_strength_spx_50(const std::vector<double>& prices, const std::vector<double>& spx_prices);
    static std::vector<double> relative_strength_sector_50(const std::vector<double>& prices, const std::vector<double>& sector_prices);
    static std::vector<double> beta_to_market_60(const std::vector<double>& returns, const std::vector<double>& market_returns);
    static std::vector<double> correlation_to_sector_40(const std::vector<double>& returns, const std::vector<double>& sector_returns);
    static std::vector<double> cross_sectional_momentum_rank_20(const std::vector<double>& returns);
    static std::vector<double> pair_spread_vs_competitor_A_30(const std::vector<double>& prices, const std::vector<double>& competitor_prices);

    // Regime Detection
    static std::vector<double> markov_regime_switching_garch_2_state(const std::vector<double>& returns);
    static std::vector<double> adx_rating_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close);
    static std::vector<double> chow_test_statistic_breakpoint_detection_50(const std::vector<double>& returns);
    static std::vector<double> market_regime_hmm_3_states_price_vol(const std::vector<double>& prices, const std::vector<double>& volatility);
    static std::vector<double> high_volatility_indicator_garch_threshold(const std::vector<double>& returns, double threshold);

    // Market Microstructure
    static std::vector<double> bid_ask_spread_volatility_10(const std::vector<double>& bid_ask_spread);
    static std::vector<double> order_flow_imbalance_5(const std::vector<double>& uptick_volume, const std::vector<double>& downtick_volume);
    static std::vector<double> price_impact_of_volume_spike(const std::vector<double>& prices, const std::vector<double>& volume);
    static std::vector<double> probability_of_informed_trading_pin(const std::vector<double>& buy_volume, const std::vector<double>& sell_volume);
    static std::vector<double> effective_spread_5_min_avg(const std::vector<double>& effective_spread);

    // Volatility Surface
    static std::vector<double> implied_volatility_atm_30d(const std::vector<double>& iv_data);
    static std::vector<double> implied_volatility_skew_25d(const std::vector<double>& put_iv, const std::vector<double>& call_iv);
    static std::vector<double> implied_volatility_term_structure_slope_30d_90d(const std::vector<double>& iv_30d, const std::vector<double>& iv_90d);
    static std::vector<double> realized_vs_implied_volatility_ratio_20(const std::vector<double>& realized_vol, const std::vector<double>& implied_vol);

    // Factor Exposures
    static std::vector<double> fama_french_smb_beta_120(const std::vector<double>& returns, const std::vector<double>& smb_returns);
    static std::vector<double> fama_french_hml_beta_120(const std::vector<double>& returns, const std::vector<double>& hml_returns);
    static std::vector<double> momentum_factor_beta_120(const std::vector<double>& returns, const std::vector<double>& momentum_returns);
    static std::vector<double> quality_factor_beta_120(const std::vector<double>& returns, const std::vector<double>& quality_returns);

    // Non-Linear/Interaction
    static std::vector<double> return_x_volume_interaction_10(const std::vector<double>& returns, const std::vector<double>& volume);
    static std::vector<double> volatility_x_rsi_interaction_14(const std::vector<double>& volatility, const std::vector<double>& rsi);
    static std::vector<double> price_to_kama_ratio_20_10_30(const std::vector<double>& prices);
    static std::vector<double> polynomial_regression_price_degree_2_slope(const std::vector<double>& prices, int window);

    // Alternative Risk Measures
    static std::vector<double> conditional_value_at_risk_cvar_95_20(const std::vector<double>& returns);
    static std::vector<double> drawdown_duration_from_peak_50(const std::vector<double>& prices);
    static std::vector<double> ulcer_index_14(const std::vector<double>& prices);
    static std::vector<double> sortino_ratio_30(const std::vector<double>& returns);

private:
    static double calculate_atr(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, int period, size_t index);
    static std::vector<double> exponential_moving_average(const std::vector<double>& data, int period);
};
