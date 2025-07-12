#pragma once

#include <vector>
#include <immintrin.h>
#include <cmath>
#include <algorithm>
#include <numeric>

#ifdef __AVX2__
#define SIMD_ENABLED 1
#else
#define SIMD_ENABLED 0
#warning "AVX2 not available. Falling back to scalar implementation."
#endif

class SIMDTechnicalIndicators {
private:
    // SIMD constants
    static constexpr size_t AVX2_FLOAT_COUNT = 8;  // 8 floats per 256-bit register
    static constexpr size_t AVX2_DOUBLE_COUNT = 4; // 4 doubles per 256-bit register
    
    // Memory alignment helpers
    static bool is_aligned(const void* ptr, size_t alignment = 32) {
        return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
    }
    
    // SIMD-optimized helper functions
    static std::vector<double> simd_subtract_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> simd_divide_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> simd_log_array(const std::vector<double>& data);
    static std::vector<double> simd_sqrt_array(const std::vector<double>& data);
    static double simd_dot_product(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> simd_rolling_sum(const std::vector<double>& data, size_t window);

public:
    // Vectorized implementations of core technical indicators
    static std::vector<double> calculate_returns_simd(const std::vector<double>& prices);
    static std::vector<double> simple_moving_average_simd(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_rolling_volatility_simd(const std::vector<double>& returns, int window);
    static std::vector<double> calculate_momentum_simd(const std::vector<double>& prices, int period);
    static std::vector<double> compute_spread_simd(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> linear_slope_simd(const std::vector<double>& prices, int window_size);
    static std::vector<double> log_pct_change_simd(const std::vector<double>& prices, int window_size);
    static std::vector<double> skewness_simd(const std::vector<double>& prices, int window_size);
    static std::vector<double> kurtosis_simd(const std::vector<double>& prices, int window_size);
    
    // Parallel processing utilities
    static void process_multiple_series_parallel(
        const std::vector<std::vector<double>>& input_series,
        std::vector<std::vector<double>>& output_results,
        std::function<std::vector<double>(const std::vector<double>&)> processor
    );
};