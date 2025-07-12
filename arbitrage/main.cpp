#include "include/core/arbitrage_analyzer.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <thread>

void printBanner() {
    std::cout << "=========================================" << std::endl;
    std::cout << "  MFT Statistical Arbitrage Analyzer   " << std::endl;
    std::cout << "  High-Performance Pair Analysis Tool  " << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void printSystemInfo() {
    std::cout << "=== SYSTEM INFORMATION ===" << std::endl;
    std::cout << "CPU Cores: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "SIMD Support:" << std::endl;
    std::cout << "  - AVX2: " << (SIMDStatistics::isAVX2Available() ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "  - NEON: " << (SIMDStatistics::isNEONAvailable() ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;
}

void printProgressCallback(const std::string& stage, double progress_percent) {
    static std::string last_stage;
    static auto last_update = std::chrono::steady_clock::now();
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);
    
    // Update progress every 500ms or when stage changes
    if (stage != last_stage || elapsed.count() > 500) {
        std::cout << "\r[" << stage << "] " << std::fixed << std::setprecision(1) 
                  << progress_percent << "% complete" << std::flush;
        
        if (stage != last_stage) {
            std::cout << std::endl;
        }
        
        last_stage = stage;
        last_update = now;
    }
}

void printAnalysisResults(const ArbitrageAnalyzer::AnalysisMetrics& metrics) {
    std::cout << std::endl;
    std::cout << "=== ANALYSIS RESULTS ===" << std::endl;
    
    // Data loading results
    std::cout << "Data Loading:" << std::endl;
    std::cout << "  - Stocks loaded: " << metrics.stocks_loaded << std::endl;
    std::cout << "  - Stocks filtered: " << metrics.stocks_filtered << std::endl;
    std::cout << "  - Loading time: " << std::fixed << std::setprecision(3) 
              << metrics.loading_time_seconds << " seconds" << std::endl;
    
    // Analysis results
    std::cout << "Pair Analysis:" << std::endl;
    std::cout << "  - Total pairs analyzed: " << metrics.total_pairs_analyzed << std::endl;
    std::cout << "  - Cointegrated pairs found: " << metrics.cointegrated_pairs_found << std::endl;
    std::cout << "  - High correlation pairs found: " << metrics.high_correlation_pairs_found << std::endl;
    std::cout << "  - Arbitrage opportunities found: " << metrics.arbitrage_opportunities_found << std::endl;
    std::cout << "  - Analysis time: " << std::fixed << std::setprecision(3) 
              << metrics.analysis_time_seconds << " seconds" << std::endl;
    
    // Performance metrics
    std::cout << "Performance:" << std::endl;
    std::cout << "  - Pairs per second: " << std::fixed << std::setprecision(2) 
              << metrics.pairs_per_second << std::endl;
    std::cout << "  - GFLOPS achieved: " << std::fixed << std::setprecision(3) 
              << metrics.gflops_achieved << std::endl;
    std::cout << "  - SIMD type used: " << metrics.simd_type_used << std::endl;
    
    // Export results
    std::cout << "Export:" << std::endl;
    std::cout << "  - Export time: " << std::fixed << std::setprecision(3) 
              << metrics.export_time_seconds << " seconds" << std::endl;
    std::cout << "  - Export successful: " << (metrics.export_successful ? "YES" : "NO") << std::endl;
    
    // Overall metrics
    std::cout << "Overall:" << std::endl;
    std::cout << "  - Total time: " << std::fixed << std::setprecision(3) 
              << metrics.total_time_seconds << " seconds" << std::endl;
    std::cout << "  - Analysis timestamp: " << metrics.analysis_timestamp << std::endl;
    
    std::cout << "========================" << std::endl;
}

void printUsageExamples() {
    std::cout << "Usage Examples:" << std::endl;
    std::cout << "  # Run with default settings" << std::endl;
    std::cout << "  ./arbitrage_analyzer" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Specify custom data directory" << std::endl;
    std::cout << "  ./arbitrage_analyzer --input-dir /path/to/data --output-dir /path/to/output" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Focus on specific sectors" << std::endl;
    std::cout << "  ./arbitrage_analyzer --sectors Technology,Healthcare" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Adjust correlation threshold" << std::endl;
    std::cout << "  ./arbitrage_analyzer --min-correlation 0.8" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Run performance benchmark" << std::endl;
    std::cout << "  ./arbitrage_analyzer --benchmark" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Interactive configuration" << std::endl;
    std::cout << "  ./arbitrage_analyzer --interactive" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Check for help or usage
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        ArbitrageCLI::printUsage();
        printUsageExamples();
        return 0;
    }
    
    // Check for benchmark mode
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        std::cout << "=== RUNNING PERFORMANCE BENCHMARK ===" << std::endl;
        printSystemInfo();
        
        // Load sample data for benchmarking
        ArbitrageAnalyzer::AnalysisConfig benchmark_config = ArbitrageCLI::parseCommandLine(argc, argv);
        benchmark_config.max_pairs_to_analyze = 1000; // Limit for benchmark
        
        auto stocks = ArbitrageAnalyzer::loadStockData(benchmark_config);
        if (stocks.size() < 10) {
            std::cerr << "Error: Need at least 10 stocks for benchmarking" << std::endl;
            return 1;
        }
        
        // Run SIMD performance benchmark
        std::vector<StockData> stock_data_for_benchmark;
        size_t benchmark_size = std::min(size_t(10), stocks.size());
        stock_data_for_benchmark.reserve(benchmark_size);
        for (size_t i = 0; i < benchmark_size; ++i) {
            if (stocks[i]) {
                stock_data_for_benchmark.push_back(*stocks[i]);
            }
        }
        SIMDPerformanceBenchmark::compareImplementations(stock_data_for_benchmark);
        
        auto benchmark_result = SIMDPerformanceBenchmark::getLastBenchmarkResult();
        std::cout << "Benchmark Results:" << std::endl;
        std::cout << "  - Scalar time: " << benchmark_result.scalar_time_ms << " ms" << std::endl;
        std::cout << "  - SIMD time: " << benchmark_result.simd_time_ms << " ms" << std::endl;
        std::cout << "  - Speedup factor: " << benchmark_result.speedup_factor << "x" << std::endl;
        std::cout << "  - Best implementation: " << benchmark_result.best_implementation << std::endl;
        
        return 0;
    }
    
    try {
        printSystemInfo();
        
        // Parse command-line arguments or use interactive mode
        ArbitrageAnalyzer::AnalysisConfig config;
        
        if (argc > 1 && std::string(argv[1]) == "--interactive") {
            config = ArbitrageCLI::interactiveConfig();
        } else {
            config = ArbitrageCLI::parseCommandLine(argc, argv);
        }
        
        // Validate configuration
        if (!ConfigManager::validateConfig(config)) {
            std::cerr << "Error: Invalid configuration" << std::endl;
            return 1;
        }
        
        // Set up progress callback
        ArbitrageAnalyzer::setProgressCallback(printProgressCallback);
        
        // Check if input directory exists
        if (!std::filesystem::exists(config.input_data_directory)) {
            std::cerr << "Error: Input directory does not exist: " 
                      << config.input_data_directory << std::endl;
            return 1;
        }
        
        // Create output directory if it doesn't exist
        std::filesystem::create_directories(config.output_directory);
        
        std::cout << "Starting analysis with configuration:" << std::endl;
        std::cout << "  - Input directory: " << config.input_data_directory << std::endl;
        std::cout << "  - Output directory: " << config.output_directory << std::endl;
        std::cout << "  - Min correlation threshold: " << config.min_correlation_threshold << std::endl;
        std::cout << "  - Max cointegration p-value: " << config.max_cointegration_pvalue << std::endl;
        std::cout << "  - Number of threads: " << (config.num_threads == 0 ? "auto-detect" : std::to_string(config.num_threads)) << std::endl;
        std::cout << "  - SIMD enabled: " << (config.enable_simd ? "YES" : "NO") << std::endl;
        std::cout << "  - Caching enabled: " << (config.enable_caching ? "YES" : "NO") << std::endl;
        std::cout << std::endl;
        
        // Run the full analysis
        auto start_time = std::chrono::high_resolution_clock::now();
        
        bool success = ArbitrageAnalyzer::runFullAnalysis(config);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << std::endl; // Clear progress line
        
        if (success) {
            auto metrics = ArbitrageAnalyzer::getLastAnalysisMetrics();
            printAnalysisResults(metrics);
            
            std::cout << std::endl;
            std::cout << "Analysis completed successfully!" << std::endl;
            std::cout << "Results exported to: " << config.output_directory << std::endl;
            
            // Print cache statistics if caching was enabled
            if (config.enable_caching) {
                auto cache_stats = AnalysisCache::getCacheStats();
                std::cout << std::endl;
                std::cout << "Cache Statistics:" << std::endl;
                std::cout << "  - Cache hit rate: " << std::fixed << std::setprecision(1) 
                          << cache_stats.cache_hit_rate * 100 << "%" << std::endl;
                std::cout << "  - Memory used: " << cache_stats.memory_used_mb << " MB" << std::endl;
            }
            
        } else {
            std::cerr << "Analysis failed. Check error messages above." << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
