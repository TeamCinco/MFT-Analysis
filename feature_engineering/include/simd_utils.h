#pragma once
#include <vector>

#ifdef __aarch64__
#include <arm_neon.h>  // ARM NEON for Apple Silicon
#elif defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>  // Intel SIMD
#endif

class SIMDUtils {
public:
    static bool has_neon();    // For ARM
    static bool has_avx2();    // For Intel
    static std::vector<double> simd_rolling_mean(const std::vector<double>& data, size_t window);
    static bool has_simd_support(); // Add this method
    
private:
    static thread_local std::vector<double> workspace_;
};