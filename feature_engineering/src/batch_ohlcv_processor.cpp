#include "batch_ohlc_processor.h"
#include "technical_indicators.h"
#include "simd_technical_indicators.h"
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
    ComputationCache cache(SIMDTechnicalIndicators::is_simd_available() && !force_scalar);

    auto& returns = cache.get_or_compute("returns",
        [&](){ return SIMDTechnicalIndicators::calculate_returns_simd(close); },
        [&](){ return TechnicalIndicators::calculate_returns(close); }
    );
    features.returns = returns;

    if (!returns.empty()) {
        features.volatility = cache.get_or_compute("volatility",
            [&](){ return SIMDTechnicalIndicators::calculate_rolling_volatility_simd(returns, 20); },
            [&](){ return TechnicalIndicators::calculate_rolling_volatility(returns, 20); }
        );
    }

    features.sma = cache.get_or_compute("sma20",
        [&](){ return SIMDTechnicalIndicators::simple_moving_average_simd(close, 20); },
        [&](){ return TechnicalIndicators::simple_moving_average(close, 20); }
    );
    features.volume_sma_20 = cache.get_or_compute("vol_sma20",
        [&](){ return SIMDTechnicalIndicators::simple_moving_average_simd(volume, 20); },
        [&](){ return TechnicalIndicators::simple_moving_average(volume, 20); }
    );
    features.spread = cache.get_or_compute("spread",
        [&](){ return SIMDTechnicalIndicators::compute_spread_simd(high, low); },
        [&](){ return TechnicalIndicators::compute_spread(high, low); }
    );
    features.log_pct_change_5 = cache.get_or_compute("log_pct_5",
        [&](){ return SIMDTechnicalIndicators::log_pct_change_simd(close, 5); },
        [&](){ return TechnicalIndicators::log_pct_change(close, 5); }
    );
    features.linear_slope_20 = cache.get_or_compute("slope20",
        [&](){ return SIMDTechnicalIndicators::linear_slope_simd(close, 20); },
        [&](){ return TechnicalIndicators::linear_slope(close, 20); }
    );
    features.linear_slope_60 = cache.get_or_compute("slope60",
        [&](){ return SIMDTechnicalIndicators::linear_slope_simd(close, 60); },
        [&](){ return TechnicalIndicators::linear_slope(close, 60); }
    );
    // Calculate momentum (10-period price ratio)
    if (close.size() > 10) {
        features.momentum.reserve(close.size() - 10);
        for (size_t i = 10; i < close.size(); ++i) {
            features.momentum.push_back(close[i-10] != 0.0 ? close[i] / close[i-10] : 0.0);
        }
    }

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
