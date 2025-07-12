#pragma once

#include "../core/stock_data.h"
#include <vector>
#include <chrono>
#include <cmath>

// Only include SIMD headers for appropriate architectures
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>  // AVX2 for Intel/AMD
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>   // NEON for ARM
#endif

class SIMDStatistics {
public:
    // Check SIMD availability
    static bool isAVX2Available();
    static bool isNEONAvailable();
    
    // SIMD-optimized correlation calculations
    template<typename Allocator1, typename Allocator2>
    static double calculateCorrelation_SIMD(
        const std::vector<double, Allocator1>& series1,
        const std::vector<double, Allocator2>& series2
    ) {
        return calculateCorrelation_SIMD(series1.data(), series2.data(), 
                                       std::min(series1.size(), series2.size()));
    }
    
    static double calculateCorrelation_SIMD(
        const double* data1, const double* data2, size_t size
    );
    
    // Batch correlation calculation (process multiple pairs simultaneously)
    static std::vector<double> calculateBatchCorrelations_SIMD(
        const std::vector<std::vector<double>>& price_series
    );
    
    // SIMD-optimized linear regression for cointegration
    static std::pair<double, double> linearRegression_SIMD(
        const std::vector<double>& y,
        const std::vector<double>& x
    );
    
    // SIMD-optimized rolling statistics
    static std::vector<double> rollingMean_SIMD(
        const std::vector<double>& data,
        int window_size
    );
    
    static std::vector<double> rollingStdDev_SIMD(
        const std::vector<double>& data,
        int window_size
    );
    
    static std::vector<double> rollingCorrelation_SIMD(
        const std::vector<double>& series1,
        const std::vector<double>& series2,
        int window_size
    );
    
    // SIMD-optimized statistical tests
    static double augmentedDickeyFuller_SIMD(
        const std::vector<double>& series,
        int lags = 1
    );
    
    // Batch ADF tests for multiple spreads
    static std::vector<double> batchAugmentedDickeyFuller_SIMD(
        const std::vector<std::vector<double>>& spreads
    );
    
    // SIMD-optimized variance and covariance
    static double variance_SIMD(const std::vector<double>& data);
    static double covariance_SIMD(
        const std::vector<double>& series1,
        const std::vector<double>& series2
    );
    
    // Performance metrics
    struct SIMDMetrics {
        double computation_time_ms = 0.0;
        size_t operations_performed = 0;
        double gflops = 0.0;
        std::string simd_type_used; // "AVX2", "NEON", or "Scalar"
    };
    
    static SIMDMetrics getLastMetrics() { return last_metrics_; }
    
    // Scalar fallback implementations (made public for benchmarking)
    static double calculateCorrelation_Scalar(
        const double* data1, const double* data2, size_t size
    );

private:
    // AVX2 implementations
    static double calculateCorrelation_AVX2(
        const double* data1, const double* data2, size_t size
    );
    
    static std::pair<double, double> linearRegression_AVX2(
        const double* y, const double* x, size_t size
    );
    
    static void rollingMean_AVX2(
        const double* input, double* output, size_t size, int window
    );
    
    static double variance_AVX2(const double* data, size_t size);
    
    // NEON implementations
#ifdef __ARM_NEON
    static double calculateCorrelation_NEON(
        const double* data1, const double* data2, size_t size
    );
    
    static std::pair<double, double> linearRegression_NEON(
        const double* y, const double* x, size_t size
    );
    
    static void rollingMean_NEON(
        const double* input, double* output, size_t size, int window
    );
    
    static double variance_NEON(const double* data, size_t size);
#endif
    
    static std::pair<double, double> linearRegression_Scalar(
        const double* y, const double* x, size_t size
    );
    
    static void rollingMean_Scalar(
        const double* input, double* output, size_t size, int window
    );
    
    static double variance_Scalar(const double* data, size_t size);
    
    // Utility functions
    static void ensureAlignment(std::vector<double>& data);
    static size_t getAlignedSize(size_t size);
    
    // Performance tracking
    static SIMDMetrics last_metrics_;
    static std::chrono::high_resolution_clock::time_point start_time_;
    
    static void startTiming();
    static void endTiming(size_t operations, const std::string& simd_type);
};

// Specialized cointegration analyzer using SIMD
class SIMDCointegrationAnalyzer {
public:
    // Perform Engle-Granger cointegration test with SIMD optimization
    static CointegrationResult analyzeCointegration_SIMD(
        const StockData& stock1,
        const StockData& stock2
    );
    
    // Batch cointegration analysis for multiple pairs
    static std::vector<CointegrationResult> batchAnalyzeCointegration_SIMD(
        const std::vector<std::pair<const StockData*, const StockData*>>& stock_pairs
    );
    
    // Calculate optimal hedge ratio using SIMD
    static double calculateHedgeRatio_SIMD(
        const std::vector<double>& prices1,
        const std::vector<double>& prices2
    );
    
    // Calculate spread statistics
    static std::tuple<double, double, double> calculateSpreadStats_SIMD(
        const std::vector<double>& spread
    ); // returns mean, std, current_value
    
    // Calculate half-life of mean reversion
    static double calculateHalfLife_SIMD(
        const std::vector<double>& spread
    );

private:
    // Critical values for ADF test at different significance levels
    static const double ADF_CRITICAL_1PCT;
    static const double ADF_CRITICAL_5PCT;
    static const double ADF_CRITICAL_10PCT;
    
    // Grade cointegration result based on statistical significance
    static std::string gradeCointegration(
        double adf_statistic, double p_value, double half_life
    );
    
    // Calculate trading metrics from historical spread
    static std::tuple<double, double, int, double> calculateTradingMetrics(
        const std::vector<double>& spread,
        double entry_threshold = 2.0,
        double exit_threshold = 0.5
    ); // returns expected_return, sharpe_ratio, num_trades, win_rate
};

// Specialized correlation analyzer using SIMD
class SIMDCorrelationAnalyzer {
public:
    // Comprehensive correlation analysis with SIMD optimization
    static CorrelationResult analyzeCorrelation_SIMD(
        const StockData& stock1,
        const StockData& stock2
    );
    
    // Batch correlation analysis for multiple pairs
    static std::vector<CorrelationResult> batchAnalyzeCorrelation_SIMD(
        const std::vector<std::pair<const StockData*, const StockData*>>& stock_pairs
    );
    
    // Calculate Spearman rank correlation using SIMD
    static double calculateSpearmanCorrelation_SIMD(
        const std::vector<double>& series1,
        const std::vector<double>& series2
    );
    
    // Calculate Kendall's tau correlation
    static double calculateKendallTau_SIMD(
        const std::vector<double>& series1,
        const std::vector<double>& series2
    );
    
    // Calculate correlation stability metric
    static double calculateCorrelationStability_SIMD(
        const std::vector<double>& rolling_correlations
    );

private:
    // Rank transformation for Spearman correlation
    static std::vector<double> rankTransform_SIMD(
        const std::vector<double>& data
    );
    
    // Grade correlation result
    static std::string gradeCorrelation(
        double pearson_corr, double stability, bool same_sector
    );
    
    // Count correlation breakdowns
    static int countCorrelationBreakdowns(
        const std::vector<double>& rolling_correlations,
        double threshold = 0.5
    );
};

// Performance benchmarking for SIMD operations
class SIMDPerformanceBenchmark {
public:
    // Benchmark different SIMD implementations
    static void benchmarkCorrelationMethods(
        const std::vector<StockData>& stocks,
        int num_iterations = 100
    );
    
    static void benchmarkCointegrationMethods(
        const std::vector<StockData>& stocks,
        int num_iterations = 10
    );
    
    // Compare SIMD vs scalar performance
    static void compareImplementations(
        const std::vector<StockData>& stocks
    );
    
    struct BenchmarkResult {
        double scalar_time_ms;
        double simd_time_ms;
        double speedup_factor;
        double gflops_scalar;
        double gflops_simd;
        std::string best_implementation;
    };
    
    static BenchmarkResult getLastBenchmarkResult() { return last_benchmark_; }

private:
    static BenchmarkResult last_benchmark_;
};
