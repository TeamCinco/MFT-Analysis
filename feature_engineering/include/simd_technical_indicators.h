#pragma once

#include <vector>
#include <functional>

#ifdef __AVX2__
#include <immintrin.h>
#define SIMD_ENABLED 1
#else
#warning "AVX2 not available. Falling back to scalar implementation."
#define SIMD_ENABLED 0
#endif

class SIMDTechnicalIndicators {
public:
#if SIMD_ENABLED
    static constexpr size_t AVX2_DOUBLE_COUNT = 4;
#endif

    static std::vector<double> calculate_returns_simd(const std::vector<double>& prices);
    static std::vector<double> simple_moving_average_simd(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_rolling_volatility_simd(const std::vector<double>& returns, int window);
    static std::vector<double> compute_spread_simd(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> linear_slope_simd(const std::vector<double>& prices, int window_size);
    static std::vector<double> log_pct_change_simd(const std::vector<double>& prices, int window_size);
    static std::vector<double> calculate_momentum_simd(const std::vector<double>& prices, int period);

    static void process_multiple_series_parallel(
        const std::vector<std::vector<double>>& input_series,
        std::vector<std::vector<double>>& output_results,
        std::function<std::vector<double>(const std::vector<double>&)> processor
    );

    static bool is_simd_available();

#if SIMD_ENABLED
    // SIMD helper functions
    static std::vector<double> simd_subtract_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> simd_divide_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static double simd_sum_array(const std::vector<double>& data, size_t start, size_t count);
    static std::vector<double> simd_rolling_sum(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_momentum_simd(const std::vector<double>& prices, int period);
#endif
};
