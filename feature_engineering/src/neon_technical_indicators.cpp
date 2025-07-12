#include "../include/neon_technical_indicators.h"
#include "../include/technical_indicators.h"
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <execution>
#include <thread>
#include <memory>

#if NEON_ENABLED
// High-performance NEON helper function implementations
std::vector<double> NEONTechnicalIndicators::neon_subtract_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};
    
    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;

    // Process in NEON chunks
    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        // Prefetch next cache lines for better memory throughput
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < size) {
            prefetch_data(&a[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
            prefetch_data(&b[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t va = vld1q_f64(&a[i]);
        float64x2_t vb = vld1q_f64(&b[i]);
        float64x2_t vresult = vsubq_f64(va, vb);
        vst1q_f64(&result[i], vresult);
    }
    
    // Handle remaining elements
    for (size_t i = neon_size; i < size; ++i) {
        result[i] = a[i] - b[i];
    }
    return result;
}

std::vector<double> NEONTechnicalIndicators::neon_divide_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};

    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;
    const float64x2_t vzero = vdupq_n_f64(0.0);

    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < size) {
            prefetch_data(&a[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
            prefetch_data(&b[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t va = vld1q_f64(&a[i]);
        float64x2_t vb = vld1q_f64(&b[i]);
        
        // Create mask for non-zero divisors
        uint64x2_t mask = vceqq_f64(vb, vzero);
        
        // Perform division
        float64x2_t vresult = vdivq_f64(va, vb);
        
        // Zero out results where divisor was zero
        vresult = vbslq_f64(mask, vzero, vresult);
        vst1q_f64(&result[i], vresult);
    }
    
    for (size_t i = neon_size; i < size; ++i) {
        result[i] = (b[i] != 0.0) ? a[i] / b[i] : 0.0;
    }
    return result;
}

std::vector<double> NEONTechnicalIndicators::neon_multiply_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};
    
    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;

    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < size) {
            prefetch_data(&a[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
            prefetch_data(&b[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t va = vld1q_f64(&a[i]);
        float64x2_t vb = vld1q_f64(&b[i]);
        float64x2_t vresult = vmulq_f64(va, vb);
        vst1q_f64(&result[i], vresult);
    }
    
    for (size_t i = neon_size; i < size; ++i) {
        result[i] = a[i] * b[i];
    }
    return result;
}

std::vector<double> NEONTechnicalIndicators::neon_add_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};
    
    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;

    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < size) {
            prefetch_data(&a[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
            prefetch_data(&b[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t va = vld1q_f64(&a[i]);
        float64x2_t vb = vld1q_f64(&b[i]);
        float64x2_t vresult = vaddq_f64(va, vb);
        vst1q_f64(&result[i], vresult);
    }
    
    for (size_t i = neon_size; i < size; ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

double NEONTechnicalIndicators::neon_sum_array(const std::vector<double>& data, size_t start, size_t count) {
    if (start + count > data.size()) return 0.0;

    double sum = 0.0;
    const size_t end = start + count;
    const size_t neon_end = start + (count / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;
    float64x2_t vsum = vdupq_n_f64(0.0);

    for (size_t i = start; i < neon_end; i += NEON_DOUBLE_COUNT) {
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < end) {
            prefetch_data(&data[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t vdata = vld1q_f64(&data[i]);
        vsum = vaddq_f64(vsum, vdata);
    }
    
    // Horizontal sum
    sum = vgetq_lane_f64(vsum, 0) + vgetq_lane_f64(vsum, 1);
    
    for (size_t i = neon_end; i < end; ++i) {
        sum += data[i];
    }
    return sum;
}

std::vector<double> NEONTechnicalIndicators::neon_rolling_sum(const std::vector<double>& data, size_t window) {
    if (data.size() < window) return {};

    std::vector<double> result;
    result.reserve(data.size() - window + 1);
    
    // Calculate first window sum using NEON
    double sum = neon_sum_array(data, 0, window);
    result.push_back(sum);
    
    // Use sliding window technique for subsequent sums
    for (size_t i = 1; i <= data.size() - window; ++i) {
        sum = sum - data[i - 1] + data[i + window - 1];
        result.push_back(sum);
    }
    return result;
}

double NEONTechnicalIndicators::neon_dot_product(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return 0.0;
    
    const size_t size = a.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;
    float64x2_t vsum = vdupq_n_f64(0.0);

    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        if (i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT < size) {
            prefetch_data(&a[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
            prefetch_data(&b[i + PREFETCH_DISTANCE * NEON_DOUBLE_COUNT]);
        }
        
        float64x2_t va = vld1q_f64(&a[i]);
        float64x2_t vb = vld1q_f64(&b[i]);
        vsum = vfmaq_f64(vsum, va, vb);  // Fused multiply-add
    }
    
    double result = vgetq_lane_f64(vsum, 0) + vgetq_lane_f64(vsum, 1);
    
    for (size_t i = neon_size; i < size; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

void NEONTechnicalIndicators::prefetch_data(const void* addr) {
    __builtin_prefetch(addr, 0, 3);  // Prefetch for read, high temporal locality
}

void* NEONTechnicalIndicators::aligned_alloc(size_t size, size_t alignment) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
}

void NEONTechnicalIndicators::aligned_free(void* ptr) {
    free(ptr);
}

// High-level indicator implementations
std::vector<double> NEONTechnicalIndicators::calculate_returns_neon(const std::vector<double>& prices) {
    if (prices.size() < 2) return {};
    
    // For small arrays, scalar is faster due to SIMD overhead
    if (prices.size() < 100) {
        std::vector<double> result;
        result.reserve(prices.size() - 1);
        for (size_t i = 1; i < prices.size(); ++i) {
            result.push_back(prices[i-1] != 0.0 ? (prices[i] - prices[i-1]) / prices[i-1] : 0.0);
        }
        return result;
    }
    
    std::vector<double> current(prices.begin() + 1, prices.end());
    std::vector<double> previous(prices.begin(), prices.end() - 1);
    auto diffs = neon_subtract_arrays(current, previous);
    return neon_divide_arrays(diffs, previous);
}

std::vector<double> NEONTechnicalIndicators::simple_moving_average_neon(const std::vector<double>& data, size_t window) {
    auto sums = neon_rolling_sum(data, window);
    if (sums.empty()) return {};

    std::vector<double> result(sums.size());
    const double inv_window = 1.0 / window;
    const float64x2_t vinv = vdupq_n_f64(inv_window);
    const size_t size = sums.size();
    const size_t neon_size = (size / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;

    for (size_t i = 0; i < neon_size; i += NEON_DOUBLE_COUNT) {
        float64x2_t vsum = vld1q_f64(&sums[i]);
        float64x2_t vresult = vmulq_f64(vsum, vinv);
        vst1q_f64(&result[i], vresult);
    }
    for (size_t i = neon_size; i < size; ++i) {
        result[i] = sums[i] * inv_window;
    }
    return result;
}

std::vector<double> NEONTechnicalIndicators::calculate_rolling_volatility_neon(const std::vector<double>& returns, int window) {
    if (returns.size() < static_cast<size_t>(window) || window <= 1) return {};

    std::vector<double> volatility;
    volatility.reserve(returns.size() - window + 1);

    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double mean = neon_sum_array(returns, i, window) / window;
        
        // Calculate variance using NEON
        double variance = 0.0;
        const size_t end = i + window;
        const size_t neon_end = i + (window / NEON_DOUBLE_COUNT) * NEON_DOUBLE_COUNT;
        const float64x2_t vmean = vdupq_n_f64(mean);
        float64x2_t vvar = vdupq_n_f64(0.0);
        
        for (size_t j = i; j < neon_end; j += NEON_DOUBLE_COUNT) {
            float64x2_t vdata = vld1q_f64(&returns[j]);
            float64x2_t vdiff = vsubq_f64(vdata, vmean);
            vvar = vfmaq_f64(vvar, vdiff, vdiff);  // Fused multiply-add
        }

        variance = vgetq_lane_f64(vvar, 0) + vgetq_lane_f64(vvar, 1);

        for (size_t j = neon_end; j < end; ++j) {
            double diff = returns[j] - mean;
            variance += diff * diff;
        }
        volatility.push_back(std::sqrt(variance / (window - 1)));
    }
    return volatility;
}

std::vector<double> NEONTechnicalIndicators::compute_spread_neon(const std::vector<double>& high, const std::vector<double>& low) {
    return neon_subtract_arrays(high, low);
}

std::vector<double> NEONTechnicalIndicators::calculate_momentum_neon(const std::vector<double>& prices, int period) {
    if (prices.size() <= static_cast<size_t>(period)) return {};
    std::vector<double> current(prices.begin() + period, prices.end());
    std::vector<double> past(prices.begin(), prices.end() - period);
    return neon_divide_arrays(current, past);
}

std::vector<double> NEONTechnicalIndicators::linear_slope_neon(const std::vector<double>& prices, int window_size) {
    if (prices.size() < static_cast<size_t>(window_size)) return {};

    std::vector<double> result;
    result.reserve(prices.size() - window_size + 1);
    
    // Pre-calculate constants
    const double sum_x = static_cast<double>(window_size * (window_size - 1)) / 2.0;
    const double sum_x2 = static_cast<double>(window_size * (window_size - 1) * (2 * window_size - 1)) / 6.0;
    const double den = window_size * sum_x2 - sum_x * sum_x;
    if (den == 0) return {};

    // Pre-calculate x values for dot product
    std::vector<double> x_values(window_size);
    std::iota(x_values.begin(), x_values.end(), 0.0);

    for (size_t i = 0; i <= prices.size() - window_size; ++i) {
        double sum_y = neon_sum_array(prices, i, window_size);
        
        // Calculate sum_xy using NEON dot product
        std::vector<double> y_window(prices.begin() + i, prices.begin() + i + window_size);
        double sum_xy = neon_dot_product(x_values, y_window);
        
        result.push_back((window_size * sum_xy - sum_x * sum_y) / den);
    }
    return result;
}

std::vector<double> NEONTechnicalIndicators::log_pct_change_neon(const std::vector<double>& prices, int window_size) {
    if (prices.size() <= static_cast<size_t>(window_size)) return {};
    std::vector<double> current(prices.begin() + window_size, prices.end());
    std::vector<double> past(prices.begin(), prices.end() - window_size);
    auto ratios = neon_divide_arrays(current, past);

    // Apply log transformation (vectorized where possible)
    for (double& r : ratios) {
        r = (r > 0) ? std::log(r) : 0.0;
    }
    return ratios;
}

std::vector<double> NEONTechnicalIndicators::skewness_neon(const std::vector<double>& prices, int window_size) {
    // For now, delegate to scalar implementation - can be optimized further
    return TechnicalIndicators::skewness(prices, window_size);
}

std::vector<double> NEONTechnicalIndicators::kurtosis_neon(const std::vector<double>& prices, int window_size) {
    // For now, delegate to scalar implementation - can be optimized further
    return TechnicalIndicators::kurtosis(prices, window_size);
}

#else 
// Fallback implementations if NEON is not enabled
std::vector<double> NEONTechnicalIndicators::calculate_returns_neon(const std::vector<double>& prices) { 
    return TechnicalIndicators::calculate_returns(prices); 
}
std::vector<double> NEONTechnicalIndicators::simple_moving_average_neon(const std::vector<double>& data, size_t window) { 
    return TechnicalIndicators::simple_moving_average(data, window); 
}
std::vector<double> NEONTechnicalIndicators::calculate_rolling_volatility_neon(const std::vector<double>& returns, int window) { 
    return TechnicalIndicators::calculate_rolling_volatility(returns, window); 
}
std::vector<double> NEONTechnicalIndicators::compute_spread_neon(const std::vector<double>& high, const std::vector<double>& low) { 
    return TechnicalIndicators::compute_spread(high, low); 
}
std::vector<double> NEONTechnicalIndicators::linear_slope_neon(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::linear_slope(prices, window_size); 
}
std::vector<double> NEONTechnicalIndicators::log_pct_change_neon(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::log_pct_change(prices, window_size); 
}
std::vector<double> NEONTechnicalIndicators::calculate_momentum_neon(const std::vector<double>& prices, int period) {
    if (prices.size() <= static_cast<size_t>(period)) return {};
    std::vector<double> result;
    result.reserve(prices.size() - period);
    for (size_t i = period; i < prices.size(); ++i) {
        result.push_back(prices[i-period] != 0.0 ? prices[i] / prices[i-period] : 0.0);
    }
    return result;
}
std::vector<double> NEONTechnicalIndicators::skewness_neon(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::skewness(prices, window_size); 
}
std::vector<double> NEONTechnicalIndicators::kurtosis_neon(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::kurtosis(prices, window_size); 
}
#endif

// Memory management functions
std::vector<double> NEONTechnicalIndicators::aligned_vector(size_t size) {
    std::vector<double> vec;
    vec.reserve(size);
    return vec;
}

bool NEONTechnicalIndicators::is_neon_available() {
    return NEON_ENABLED;
}

void NEONTechnicalIndicators::process_multiple_series_parallel_optimized(
    const std::vector<std::vector<double>>& input_series,
    std::vector<std::vector<double>>& output_results,
    std::function<std::vector<double>(const std::vector<double>&)> processor) {
    
    output_results.resize(input_series.size());
    
    // Use manual threading for better control
    const size_t num_threads = std::thread::hardware_concurrency();
    const size_t chunk_size = (input_series.size() + num_threads - 1) / num_threads;
    
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            const size_t start_idx = t * chunk_size;
            const size_t end_idx = std::min(start_idx + chunk_size, input_series.size());
            
            for (size_t i = start_idx; i < end_idx; ++i) {
                output_results[i] = processor(input_series[i]);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}
