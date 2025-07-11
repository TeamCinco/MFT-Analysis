#include "simd_utils.h"
#include <algorithm>
#include <cstring>

thread_local std::vector<double> SIMDUtils::workspace_;

bool SIMDUtils::has_neon() {
#ifdef __aarch64__
    return true;  // All Apple Silicon Macs have NEON
#else
    return false;
#endif
}

bool SIMDUtils::has_avx2() {
#if defined(__x86_64__) || defined(__i386__)
    return __builtin_cpu_supports("avx2");
#else
    return false;
#endif
}

bool SIMDUtils::has_simd_support() {
#ifdef __aarch64__
    return has_neon();
#else
    return has_avx2();
#endif
}

std::vector<double> SIMDUtils::simd_rolling_mean(const std::vector<double>& data, size_t window) {
    std::vector<double> result;
    if (data.size() < window) return result;
    
    // For now, use scalar implementation
    // We can add ARM NEON optimizations later
    double sum = 0.0;
    for (size_t i = 0; i < window; ++i) {
        sum += data[i];
    }
    result.push_back(sum / window);
    
    for (size_t i = window; i < data.size(); ++i) {
        sum = sum - data[i - window] + data[i];
        result.push_back(sum / window);
    }
    return result;
}