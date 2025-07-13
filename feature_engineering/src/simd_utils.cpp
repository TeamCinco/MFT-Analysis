#include "../include/simd_technical_indicators.h"
#include "../include/technical_indicators.h"
#include <stdexcept>
#include <numeric>
#include <execution>
#include <cmath>

// This file contains utility functions for SIMD operations
// Main implementations are in simd_technical_indicators.cpp

void SIMDTechnicalIndicators::process_multiple_series_parallel(
    const std::vector<std::vector<double>>& input_series,
    std::vector<std::vector<double>>& output_results,
    std::function<std::vector<double>(const std::vector<double>&)> processor) {
    output_results.resize(input_series.size());
    std::transform(std::execution::par_unseq, input_series.begin(), input_series.end(), output_results.begin(), processor);
}
