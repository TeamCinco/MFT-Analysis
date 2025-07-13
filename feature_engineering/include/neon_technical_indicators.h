#pragma once

#include <vector>
#include <functional>

#ifdef __ARM_NEON
#include <arm_neon.h>
#define NEON_ENABLED 1
#else
#define NEON_ENABLED 0
#ifdef _MSC_VER
#pragma message("ARM NEON not available. Falling back to scalar implementation.")
#else
#warning "ARM NEON not available. Falling back to scalar implementation."
#endif
#endif

class NEONTechnicalIndicators {
public:
    static constexpr size_t CACHE_LINE_SIZE = 64;   // Standard cache line size
    
#if NEON_ENABLED
    static constexpr size_t NEON_DOUBLE_COUNT = 2;  // 2 doubles per 128-bit register
    static constexpr size_t NEON_FLOAT_COUNT = 4;   // 4 floats per 128-bit register
    static constexpr size_t PREFETCH_DISTANCE = 8;  // Prefetch 8 cache lines ahead
#endif

    // High-performance NEON implementations
    static std::vector<double> calculate_returns_neon(const std::vector<double>& prices);
    static std::vector<double> simple_moving_average_neon(const std::vector<double>& data, size_t window);
    static std::vector<double> calculate_rolling_volatility_neon(const std::vector<double>& returns, int window);
    static std::vector<double> compute_spread_neon(const std::vector<double>& high, const std::vector<double>& low);
    static std::vector<double> linear_slope_neon(const std::vector<double>& prices, int window_size);
    static std::vector<double> log_pct_change_neon(const std::vector<double>& prices, int window_size);
    static std::vector<double> calculate_momentum_neon(const std::vector<double>& prices, int period);
    static std::vector<double> skewness_neon(const std::vector<double>& prices, int window_size);
    static std::vector<double> kurtosis_neon(const std::vector<double>& prices, int window_size);

    // Parallel batch processing with optimal memory access patterns
    static void process_multiple_series_parallel_optimized(
        const std::vector<std::vector<double>>& input_series,
        std::vector<std::vector<double>>& output_results,
        std::function<std::vector<double>(const std::vector<double>&)> processor
    );

    // Memory-optimized operations
    static std::vector<double> aligned_vector(size_t size);
    static bool is_neon_available();

#if NEON_ENABLED
    // Low-level NEON helper functions
    static std::vector<double> neon_subtract_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> neon_divide_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> neon_multiply_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> neon_add_arrays(const std::vector<double>& a, const std::vector<double>& b);
    static double neon_sum_array(const std::vector<double>& data, size_t start, size_t count);
    static std::vector<double> neon_rolling_sum(const std::vector<double>& data, size_t window);
    static double neon_dot_product(const std::vector<double>& a, const std::vector<double>& b);
    
    // Cache-optimized memory operations
    static void prefetch_data(const void* addr);
    static void* aligned_alloc(size_t size, size_t alignment = CACHE_LINE_SIZE);
    static void aligned_free(void* ptr);
#endif

private:
    // Memory alignment helpers
    static bool is_aligned(const void* ptr, size_t alignment = CACHE_LINE_SIZE) {
        return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
    }
};
