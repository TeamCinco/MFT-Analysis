#include "batch_ohlc_processor.h"
#include "technical_indicators.h"
#include "simd_technical_indicators.h"
#include "neon_technical_indicators.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <execution>
#include <thread>

BatchOHLCProcessor::ComputationCache::ComputationCache(bool enable_simd) : use_simd_(enable_simd) {}

std::vector<double>& BatchOHLCProcessor::ComputationCache::get_or_compute(
    const std::string& key,
    std::function<std::vector<double>()> simd_computer,
    std::function<std::vector<double>()> scalar_computer) {
    if (cache_.count(key)) return cache_.at(key);
    auto result = use_simd_ ? simd_computer() : scalar_computer();
    auto [it, success] = cache_.emplace(key, std::move(result));
    return it->second;
}

void BatchOHLCProcessor::ComputationCache::clear() { cache_.clear(); }

FeatureSet BatchOHLCProcessor::calculate_features(
    const std::vector<double>& open, const std::vector<double>& high,
    const std::vector<double>& low, const std::vector<double>& close,
    const std::vector<double>& volume, bool force_scalar
) {
    if (close.empty()) throw std::runtime_error("Input vectors cannot be empty.");

    FeatureSet features;
    // Use NEON if available, otherwise fall back to AVX2, then scalar
    const bool use_neon = NEONTechnicalIndicators::is_neon_available() && !force_scalar;
    const bool use_simd = SIMDTechnicalIndicators::is_simd_available() && !force_scalar && !use_neon;
    ComputationCache cache(use_neon || use_simd);

    auto& returns = cache.get_or_compute("returns",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::calculate_returns_neon(close);
            else if (use_simd) return SIMDTechnicalIndicators::calculate_returns_simd(close);
            else return TechnicalIndicators::calculate_returns(close);
        },
        [&](){ return TechnicalIndicators::calculate_returns(close); }
    );
    features.returns = returns;

    if (!returns.empty()) {
        features.volatility = cache.get_or_compute("volatility",
            [&](){ 
                if (use_neon) return NEONTechnicalIndicators::calculate_rolling_volatility_neon(returns, 20);
                else if (use_simd) return SIMDTechnicalIndicators::calculate_rolling_volatility_simd(returns, 20);
                else return TechnicalIndicators::calculate_rolling_volatility(returns, 20);
            },
            [&](){ return TechnicalIndicators::calculate_rolling_volatility(returns, 20); }
        );
    }

    features.sma = cache.get_or_compute("sma20",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::simple_moving_average_neon(close, 20);
            else if (use_simd) return SIMDTechnicalIndicators::simple_moving_average_simd(close, 20);
            else return TechnicalIndicators::simple_moving_average(close, 20);
        },
        [&](){ return TechnicalIndicators::simple_moving_average(close, 20); }
    );
    features.volume_sma_20 = cache.get_or_compute("vol_sma20",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::simple_moving_average_neon(volume, 20);
            else if (use_simd) return SIMDTechnicalIndicators::simple_moving_average_simd(volume, 20);
            else return TechnicalIndicators::simple_moving_average(volume, 20);
        },
        [&](){ return TechnicalIndicators::simple_moving_average(volume, 20); }
    );
    features.spread = cache.get_or_compute("spread",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::compute_spread_neon(high, low);
            else if (use_simd) return SIMDTechnicalIndicators::compute_spread_simd(high, low);
            else return TechnicalIndicators::compute_spread(high, low);
        },
        [&](){ return TechnicalIndicators::compute_spread(high, low); }
    );
    features.log_pct_change_5 = cache.get_or_compute("log_pct_5",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::log_pct_change_neon(close, 5);
            else if (use_simd) return SIMDTechnicalIndicators::log_pct_change_simd(close, 5);
            else return TechnicalIndicators::log_pct_change(close, 5);
        },
        [&](){ return TechnicalIndicators::log_pct_change(close, 5); }
    );
    features.linear_slope_20 = cache.get_or_compute("slope20",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::linear_slope_neon(close, 20);
            else if (use_simd) return SIMDTechnicalIndicators::linear_slope_simd(close, 20);
            else return TechnicalIndicators::linear_slope(close, 20);
        },
        [&](){ return TechnicalIndicators::linear_slope(close, 20); }
    );
    features.linear_slope_60 = cache.get_or_compute("slope60",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::linear_slope_neon(close, 60);
            else if (use_simd) return SIMDTechnicalIndicators::linear_slope_simd(close, 60);
            else return TechnicalIndicators::linear_slope(close, 60);
        },
        [&](){ return TechnicalIndicators::linear_slope(close, 60); }
    );
    
    // Calculate momentum using optimized implementation
    features.momentum = cache.get_or_compute("momentum10",
        [&](){ 
            if (use_neon) return NEONTechnicalIndicators::calculate_momentum_neon(close, 10);
            else if (use_simd) return SIMDTechnicalIndicators::calculate_momentum_simd(close, 10);
            else {
                std::vector<double> result;
                if (close.size() > 10) {
                    result.reserve(close.size() - 10);
                    for (size_t i = 10; i < close.size(); ++i) {
                        result.push_back(close[i-10] != 0.0 ? close[i] / close[i-10] : 0.0);
                    }
                }
                return result;
            }
        },
        [&](){ 
            std::vector<double> result;
            if (close.size() > 10) {
                result.reserve(close.size() - 10);
                for (size_t i = 10; i < close.size(); ++i) {
                    result.push_back(close[i-10] != 0.0 ? close[i] / close[i-10] : 0.0);
                }
            }
            return result;
        }
    );

    features.rsi = TechnicalIndicators::calculate_rsi(close, 14);
    features.internal_bar_strength = TechnicalIndicators::internal_bar_strength(open, high, low, close);
    auto candle_info = TechnicalIndicators::candle_information(open, high, low, close);
    features.candle_way = std::move(candle_info.first);
    features.candle_filling = std::move(candle_info.second.first);
    features.candle_amplitude = std::move(candle_info.second.second);
    auto derivatives_pair = TechnicalIndicators::derivatives(close);
    features.velocity = std::move(derivatives_pair.first);
    features.acceleration = std::move(derivatives_pair.second);
    features.skewness_30 = TechnicalIndicators::skewness(close, 30);
    features.kurtosis_30 = TechnicalIndicators::kurtosis(close, 30);
    features.auto_correlation_50_10 = TechnicalIndicators::auto_correlation(close, 50, 10);
    features.kama_10_2_30 = TechnicalIndicators::kama(close, 10, 2, 30);
    features.parkinson_volatility_20 = TechnicalIndicators::parkinson_volatility(high, low, 20);

    // Statistical/Mathematical Features
    features.z_score_20 = TechnicalIndicators::z_score_20(returns);
    features.percentile_rank_50 = TechnicalIndicators::percentile_rank_50(close);
    features.coefficient_of_variation_30 = TechnicalIndicators::coefficient_of_variation_30(returns);
    features.detrended_price_oscillator_20 = TechnicalIndicators::detrended_price_oscillator_20(close);
    features.hurst_exponent_100 = TechnicalIndicators::hurst_exponent_100(close);
    features.garch_volatility_21 = TechnicalIndicators::garch_volatility_21(returns);
    features.shannon_entropy_volume_10 = TechnicalIndicators::shannon_entropy_volume_10(volume);

    // Technical Analysis Extended
    features.chande_momentum_oscillator_14 = TechnicalIndicators::chande_momentum_oscillator_14(close);
    features.aroon_oscillator_25 = TechnicalIndicators::aroon_oscillator_25(high, low);
    features.trix_15 = TechnicalIndicators::trix_15(close);
    features.vortex_indicator_14 = TechnicalIndicators::vortex_indicator_14(high, low, close);
    features.supertrend_10_3 = TechnicalIndicators::supertrend_10_3(high, low, close);
    features.ichimoku_senkou_span_A_9_26 = TechnicalIndicators::ichimoku_senkou_span_A_9_26(high, low);
    features.ichimoku_senkou_span_B_26_52 = TechnicalIndicators::ichimoku_senkou_span_B_26_52(high, low);
    features.fisher_transform_10 = TechnicalIndicators::fisher_transform_10(high, low);

    // Volume/Liquidity Advanced
    features.volume_weighted_average_price_intraday = TechnicalIndicators::volume_weighted_average_price_intraday(high, low, close, volume);
    features.volume_profile_high_volume_node_intraday = TechnicalIndicators::volume_profile_high_volume_node_intraday(close, volume);
    features.volume_profile_low_volume_node_intraday = TechnicalIndicators::volume_profile_low_volume_node_intraday(close, volume);
    features.on_balance_volume_sma_20 = TechnicalIndicators::on_balance_volume_sma_20(close, volume);
    features.klinger_oscillator_34_55 = TechnicalIndicators::klinger_oscillator_34_55(high, low, close, volume);
    features.money_flow_index_14 = TechnicalIndicators::money_flow_index_14(high, low, close, volume);
    features.vwap_deviation_stddev_30 = TechnicalIndicators::vwap_deviation_stddev_30(high, low, close, volume);

    // Regime Detection
    features.markov_regime_switching_garch_2_state = TechnicalIndicators::markov_regime_switching_garch_2_state(returns);
    features.adx_rating_14 = TechnicalIndicators::adx_rating_14(high, low, close);
    features.chow_test_statistic_breakpoint_detection_50 = TechnicalIndicators::chow_test_statistic_breakpoint_detection_50(returns);
    features.market_regime_hmm_3_states_price_vol = TechnicalIndicators::market_regime_hmm_3_states_price_vol(close, features.volatility);
    features.high_volatility_indicator_garch_threshold = TechnicalIndicators::high_volatility_indicator_garch_threshold(returns, 0.02);

    // Non-Linear/Interaction
    features.return_x_volume_interaction_10 = TechnicalIndicators::return_x_volume_interaction_10(returns, volume);
    features.volatility_x_rsi_interaction_14 = TechnicalIndicators::volatility_x_rsi_interaction_14(features.volatility, features.rsi);
    features.price_to_kama_ratio_20_10_30 = TechnicalIndicators::price_to_kama_ratio_20_10_30(close);
    features.polynomial_regression_price_degree_2_slope = TechnicalIndicators::polynomial_regression_price_degree_2_slope(close, 20);

    // Alternative Risk Measures
    features.conditional_value_at_risk_cvar_95_20 = TechnicalIndicators::conditional_value_at_risk_cvar_95_20(returns);
    features.drawdown_duration_from_peak_50 = TechnicalIndicators::drawdown_duration_from_peak_50(close);
    features.ulcer_index_14 = TechnicalIndicators::ulcer_index_14(close);
    features.sortino_ratio_30 = TechnicalIndicators::sortino_ratio_30(returns);

    return features;
}

std::vector<FeatureSet> BatchOHLCProcessor::batch_calculate_features(
    const std::vector<std::vector<double>>& open_prices,
    const std::vector<std::vector<double>>& high_prices,
    const std::vector<std::vector<double>>& low_prices,
    const std::vector<std::vector<double>>& close_prices,
    const std::vector<std::vector<double>>& volumes,
    bool force_scalar
) {
    if (close_prices.empty()) return {};
    std::vector<FeatureSet> results(close_prices.size());
    std::vector<size_t> indices(close_prices.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::for_each(indices.begin(), indices.end(),
        [&](size_t i) {
            results[i] = calculate_features(
                open_prices[i], high_prices[i], low_prices[i],
                close_prices[i], volumes[i], force_scalar
            );
        });
    return results;
}
