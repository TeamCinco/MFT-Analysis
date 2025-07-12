#include "../include/simd_technical_indicators.h"
#include "../include/technical_indicators.h"
#include <stdexcept>
#include <numeric>
#include <cmath>

#if SIMD_ENABLED
// SIMD helper function implementations
std::vector<double> SIMDTechnicalIndicators::simd_subtract_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};
    
    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t simd_size = (size / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;

    for (size_t i = 0; i < simd_size; i += AVX2_DOUBLE_COUNT) {
        // Prefetch can be added here if arrays are very large
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        __m256d vresult = _mm256_sub_pd(va, vb);
        _mm256_storeu_pd(&result[i], vresult);
    }
    for (size_t i = simd_size; i < size; ++i) result[i] = a[i] - b[i];
    return result;
}

std::vector<double> SIMDTechnicalIndicators::simd_divide_arrays(
    const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return {};

    std::vector<double> result(a.size());
    const size_t size = a.size();
    const size_t simd_size = (size / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;
    const __m256d vzero = _mm256_setzero_pd();

    for (size_t i = 0; i < simd_size; i += AVX2_DOUBLE_COUNT) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        // Avoid division by zero by creating a mask
        __m256d mask = _mm256_cmp_pd(vb, vzero, _CMP_NEQ_OQ);
        __m256d vresult = _mm256_div_pd(va, vb);
        // Use mask to zero out results where divisor was zero
        vresult = _mm256_and_pd(vresult, mask);
        _mm256_storeu_pd(&result[i], vresult);
    }
    for (size_t i = simd_size; i < size; ++i) result[i] = (b[i] != 0.0) ? a[i] / b[i] : 0.0;
    return result;
}

double SIMDTechnicalIndicators::simd_sum_array(const std::vector<double>& data, size_t start, size_t count) {
    if (start + count > data.size()) return 0.0;

    double sum = 0.0;
    const size_t simd_end = start + (count / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;
    __m256d vsum = _mm256_setzero_pd();

    for (size_t i = start; i < simd_end; i += AVX2_DOUBLE_COUNT) {
        __m256d vdata = _mm256_loadu_pd(&data[i]);
        vsum = _mm256_add_pd(vsum, vdata);
    }
    
    // Horizontal sum
    __m128d vlow = _mm256_castpd256_pd128(vsum);
    __m128d vhigh = _mm256_extractf128_pd(vsum, 1);
    vlow = _mm_add_pd(vlow, vhigh);
    __m128d high64 = _mm_unpackhi_pd(vlow, vlow);
    sum = _mm_cvtsd_f64(_mm_add_sd(vlow, high64));
    
    for (size_t i = simd_end - start; i < count; ++i) sum += data[start + i];
    return sum;
}

std::vector<double> SIMDTechnicalIndicators::simd_rolling_sum(const std::vector<double>& data, size_t window) {
    if (data.size() < window) return {};

    std::vector<double> result;
    result.reserve(data.size() - window + 1);
    
    double sum = simd_sum_array(data, 0, window);
    result.push_back(sum);
    
    for (size_t i = 1; i <= data.size() - window; ++i) {
        sum = sum - data[i - 1] + data[i + window - 1];
        result.push_back(sum);
    }
    return result;
}

// High-level indicator implementations
std::vector<double> SIMDTechnicalIndicators::calculate_returns_simd(const std::vector<double>& prices) {
    if (prices.size() < 2) return {};
    std::vector<double> current(prices.begin() + 1, prices.end());
    std::vector<double> previous(prices.begin(), prices.end() - 1);
    auto diffs = simd_subtract_arrays(current, previous);
    return simd_divide_arrays(diffs, previous);
}

std::vector<double> SIMDTechnicalIndicators::simple_moving_average_simd(const std::vector<double>& data, size_t window) {
    auto sums = simd_rolling_sum(data, window);
    if (sums.empty()) return {};

    std::vector<double> result(sums.size());
    const double inv_window = 1.0 / window;
    const __m256d vinv = _mm256_set1_pd(inv_window);
    const size_t size = sums.size();
    const size_t simd_size = (size / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;

    for (size_t i = 0; i < simd_size; i += AVX2_DOUBLE_COUNT) {
        __m256d vsum = _mm256_loadu_pd(&sums[i]);
        __m256d vresult = _mm256_mul_pd(vsum, vinv);
        _mm256_storeu_pd(&result[i], vresult);
    }
    for (size_t i = simd_size; i < size; ++i) result[i] = sums[i] * inv_window;
    return result;
}

std::vector<double> SIMDTechnicalIndicators::calculate_rolling_volatility_simd(const std::vector<double>& returns, int window) {
    if (returns.size() < static_cast<size_t>(window) || window <=1) return {};

    std::vector<double> volatility;
    volatility.reserve(returns.size() - window + 1);

    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double mean = simd_sum_array(returns, i, window) / window;
        
        double variance = 0.0;
        const size_t simd_end = i + (window / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;
        const __m256d vmean = _mm256_set1_pd(mean);
        __m256d vvar = _mm256_setzero_pd();
        
        for (size_t j = i; j < simd_end; j += AVX2_DOUBLE_COUNT) {
            __m256d vdata = _mm256_loadu_pd(&returns[j]);
            __m256d vdiff = _mm256_sub_pd(vdata, vmean);
            vvar = _mm256_fmadd_pd(vdiff, vdiff, vvar); // Fused multiply-add
        }

        double temp[4];
        _mm256_storeu_pd(temp, vvar);
        variance = temp[0] + temp[1] + temp[2] + temp[3];

        for (size_t j = simd_end - i; j < static_cast<size_t>(window); ++j) {
            double diff = returns[i + j] - mean;
            variance += diff * diff;
        }
        volatility.push_back(std::sqrt(variance / (window - 1)));
    }
    return volatility;
}

std::vector<double> SIMDTechnicalIndicators::compute_spread_simd(const std::vector<double>& high, const std::vector<double>& low) {
    return simd_subtract_arrays(high, low);
}


std::vector<double> SIMDTechnicalIndicators::linear_slope_simd(const std::vector<double>& prices, int window_size) {
     if (prices.size() < static_cast<size_t>(window_size)) return {};

    std::vector<double> result;
    result.reserve(prices.size() - window_size + 1);
    
    const double sum_x = static_cast<double>(window_size * (window_size - 1)) / 2.0;
    const double sum_x2 = static_cast<double>(window_size * (window_size - 1) * (2 * window_size - 1)) / 6.0;
    const double den = window_size * sum_x2 - sum_x * sum_x;
    if (den == 0) return {};

    for (size_t i = 0; i <= prices.size() - window_size; ++i) {
        double sum_y = simd_sum_array(prices, i, window_size);
        double sum_xy = 0.0;
        
        const size_t simd_end = i + (window_size / AVX2_DOUBLE_COUNT) * AVX2_DOUBLE_COUNT;
        __m256d vsum_xy = _mm256_setzero_pd();
        for (size_t j = 0; j < (simd_end-i); j += AVX2_DOUBLE_COUNT) {
            __m256d vy = _mm256_loadu_pd(&prices[i + j]);
            __m256d vx = _mm256_set_pd(j + 3, j + 2, j + 1, j);
            vsum_xy = _mm256_fmadd_pd(vx, vy, vsum_xy);
        }
        
        double temp[4];
        _mm256_storeu_pd(temp, vsum_xy);
        sum_xy = temp[0] + temp[1] + temp[2] + temp[3];
        
        for (size_t j = simd_end-i; j < static_cast<size_t>(window_size); ++j) {
            sum_xy += j * prices[i + j];
        }
        
        result.push_back((window_size * sum_xy - sum_x * sum_y) / den);
    }
    return result;
}

std::vector<double> SIMDTechnicalIndicators::log_pct_change_simd(const std::vector<double>& prices, int window_size) {
    if (prices.size() <= static_cast<size_t>(window_size)) return {};
    std::vector<double> current(prices.begin() + window_size, prices.end());
    std::vector<double> past(prices.begin(), prices.end() - window_size);
    auto ratios = simd_divide_arrays(current, past);

    // Note: Log operation itself is not trivially vectorized in AVX2.
    // For extreme performance, a library like Intel's SVML would be needed.
    // Here, we vectorize the division which is often the bottleneck.
    for(double& r : ratios) {
        r = (r > 0) ? std::log(r) : 0.0;
    }
    return ratios;
}

std::vector<double> SIMDTechnicalIndicators::calculate_momentum_simd(const std::vector<double>& prices, int period) {
    if (prices.size() <= static_cast<size_t>(period)) return {};
    std::vector<double> current(prices.begin() + period, prices.end());
    std::vector<double> past(prices.begin(), prices.end() - period);
    return simd_divide_arrays(current, past);
}

#else 
// Fallback implementations if SIMD is not enabled - delegate to scalar implementations
std::vector<double> SIMDTechnicalIndicators::calculate_returns_simd(const std::vector<double>& prices) { 
    return TechnicalIndicators::calculate_returns(prices); 
}
std::vector<double> SIMDTechnicalIndicators::simple_moving_average_simd(const std::vector<double>& data, size_t window) { 
    return TechnicalIndicators::simple_moving_average(data, window); 
}
std::vector<double> SIMDTechnicalIndicators::calculate_rolling_volatility_simd(const std::vector<double>& returns, int window) { 
    return TechnicalIndicators::calculate_rolling_volatility(returns, window); 
}
std::vector<double> SIMDTechnicalIndicators::compute_spread_simd(const std::vector<double>& high, const std::vector<double>& low) { 
    return TechnicalIndicators::compute_spread(high, low); 
}
std::vector<double> SIMDTechnicalIndicators::linear_slope_simd(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::linear_slope(prices, window_size); 
}
std::vector<double> SIMDTechnicalIndicators::log_pct_change_simd(const std::vector<double>& prices, int window_size) { 
    return TechnicalIndicators::log_pct_change(prices, window_size); 
}
std::vector<double> SIMDTechnicalIndicators::calculate_momentum_simd(const std::vector<double>& prices, int period) {
    if (prices.size() <= static_cast<size_t>(period)) return {};
    std::vector<double> result;
    result.reserve(prices.size() - period);
    for (size_t i = period; i < prices.size(); ++i) {
        result.push_back(prices[i-period] != 0.0 ? prices[i] / prices[i-period] : 0.0);
    }
    return result;
}
#endif

// Utility function
bool SIMDTechnicalIndicators::is_simd_available() {
    return SIMD_ENABLED;
}
