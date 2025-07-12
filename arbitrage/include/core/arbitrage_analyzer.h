#pragma once

#include "stock_data.h"
#include "fast_csv_loader.h"
#include "../statistics/simd_statistics.h"
#include "../export/excel_exporter.h"
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>

// Main arbitrage analysis engine
class ArbitrageAnalyzer {
public:
    // Configuration for analysis
    struct AnalysisConfig {
        // Data source
        std::string input_data_directory = "visualization/data/";
        std::string output_directory = "arbitrage/data/output/";
        
        // Portfolio constraints
        PortfolioConstraints portfolio_constraints;
        
        // Analysis parameters
        double min_correlation_threshold = 0.7;
        double max_cointegration_pvalue = 0.05;
        int min_data_points = 100;
        bool require_same_sector = false;
        
        // Performance settings
        unsigned int num_threads = 0; // 0 = auto-detect
        bool enable_simd = true;
        bool enable_caching = true;
        
        // Output settings
        bool export_excel = true;
        bool export_csv = true;
        bool export_json = false;
        std::string output_filename = "statistical_arbitrage_opportunities";
        
        // Analysis scope
        int max_pairs_to_analyze = 0; // 0 = analyze all pairs
        std::vector<std::string> focus_sectors; // empty = all sectors
        std::vector<std::string> excluded_symbols;
    };
    
    // Main analysis function
    static bool runFullAnalysis(const AnalysisConfig& config);
    static bool runFullAnalysis(); // Overload with default config
    
    // Individual analysis components
    static std::vector<std::unique_ptr<StockData>> loadStockData(
        const AnalysisConfig& config
    );
    
    static std::vector<CointegrationResult> analyzeCointegration(
        const std::vector<std::unique_ptr<StockData>>& stocks,
        const AnalysisConfig& config
    );
    
    static std::vector<CorrelationResult> analyzeCorrelation(
        const std::vector<std::unique_ptr<StockData>>& stocks,
        const AnalysisConfig& config
    );
    
    static std::vector<ArbitrageOpportunity> generateOpportunities(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const AnalysisConfig& config
    );
    
    static bool exportResults(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const AnalysisConfig& config
    );
    
    // Performance and progress tracking
    struct AnalysisMetrics {
        // Data loading metrics
        size_t stocks_loaded = 0;
        size_t stocks_filtered = 0;
        double loading_time_seconds = 0.0;
        
        // Analysis metrics
        size_t total_pairs_analyzed = 0;
        size_t cointegrated_pairs_found = 0;
        size_t high_correlation_pairs_found = 0;
        size_t arbitrage_opportunities_found = 0;
        double analysis_time_seconds = 0.0;
        
        // Performance metrics
        double pairs_per_second = 0.0;
        double gflops_achieved = 0.0;
        std::string simd_type_used;
        
        // Export metrics
        double export_time_seconds = 0.0;
        bool export_successful = false;
        
        // Overall metrics
        double total_time_seconds = 0.0;
        std::string analysis_timestamp;
    };
    
    static AnalysisMetrics getLastAnalysisMetrics() { return last_metrics_; }
    
    // Progress callback for long-running analysis
    using ProgressCallback = std::function<void(const std::string& stage, double progress_percent)>;
    static void setProgressCallback(ProgressCallback callback) { progress_callback_ = callback; }

private:
    // Parallel analysis coordination
    static std::vector<CointegrationResult> analyzeCointegrationParallel(
        const std::vector<std::unique_ptr<StockData>>& stocks,
        const AnalysisConfig& config
    );
    
    static std::vector<CorrelationResult> analyzeCorrelationParallel(
        const std::vector<std::unique_ptr<StockData>>& stocks,
        const AnalysisConfig& config
    );
    
    // Adaptive work distribution
    static void distributePairAnalysis(
        const std::vector<std::unique_ptr<StockData>>& stocks,
        std::vector<std::pair<size_t, size_t>>& work_batches,
        unsigned int num_threads
    );
    
    // Filtering and validation
    static std::vector<std::unique_ptr<StockData>> filterStocks(
        std::vector<std::unique_ptr<StockData>> stocks,
        const AnalysisConfig& config
    );
    
    static bool isValidPair(
        const StockData& stock1,
        const StockData& stock2,
        const AnalysisConfig& config
    );
    
    // Opportunity scoring and ranking
    static double calculateCombinedScore(
        const CointegrationResult& coint_result,
        const CorrelationResult& corr_result
    );
    
    static std::string assignOpportunityGrade(double combined_score);
    
    static ArbitrageOpportunity createOpportunity(
        const CointegrationResult& coint_result,
        const CorrelationResult& corr_result,
        const AnalysisConfig& config
    );
    
    // Performance optimization
    static void optimizeForHardware();
    static unsigned int getOptimalThreadCount();
    static size_t getOptimalBatchSize();
    
    // Progress reporting
    static void reportProgress(const std::string& stage, double progress);
    
    // Metrics tracking
    static AnalysisMetrics last_metrics_;
    static ProgressCallback progress_callback_;
    static std::chrono::high_resolution_clock::time_point analysis_start_time_;
    
    // Thread synchronization
    static std::mutex progress_mutex_;
    static std::atomic<size_t> pairs_completed_;
    static std::atomic<size_t> total_pairs_;
};

// Intelligent caching system for analysis results
class AnalysisCache {
public:
    // Cache cointegration results
    static void cacheCointegrationResult(
        const std::string& symbol1,
        const std::string& symbol2,
        const CointegrationResult& result
    );
    
    static bool getCachedCointegrationResult(
        const std::string& symbol1,
        const std::string& symbol2,
        CointegrationResult& result
    );
    
    // Cache correlation results
    static void cacheCorrelationResult(
        const std::string& symbol1,
        const std::string& symbol2,
        const CorrelationResult& result
    );
    
    static bool getCachedCorrelationResult(
        const std::string& symbol1,
        const std::string& symbol2,
        CorrelationResult& result
    );
    
    // Cache management
    static void clearCache();
    static size_t getCacheSize();
    static void saveCacheToFile(const std::string& cache_file);
    static void loadCacheFromFile(const std::string& cache_file);
    
    // Cache statistics
    struct CacheStats {
        size_t cointegration_cache_hits = 0;
        size_t cointegration_cache_misses = 0;
        size_t correlation_cache_hits = 0;
        size_t correlation_cache_misses = 0;
        double cache_hit_rate = 0.0;
        size_t memory_used_mb = 0;
    };
    
    static CacheStats getCacheStats() { return cache_stats_; }

private:
    // Cache storage
    static std::unordered_map<std::string, CointegrationResult> cointegration_cache_;
    static std::unordered_map<std::string, CorrelationResult> correlation_cache_;
    
    // Cache key generation
    static std::string makePairKey(const std::string& symbol1, const std::string& symbol2);
    
    // Cache statistics
    static CacheStats cache_stats_;
    static std::mutex cache_mutex_;
};

// Configuration management
class ConfigManager {
public:
    // Load configuration from file
    static ArbitrageAnalyzer::AnalysisConfig loadConfig(const std::string& config_file);
    
    // Save configuration to file
    static bool saveConfig(
        const ArbitrageAnalyzer::AnalysisConfig& config,
        const std::string& config_file
    );
    
    // Create default configuration
    static ArbitrageAnalyzer::AnalysisConfig createDefaultConfig();
    
    // Validate configuration
    static bool validateConfig(const ArbitrageAnalyzer::AnalysisConfig& config);

private:
    // Configuration file parsing
    static std::unordered_map<std::string, std::string> parseConfigFile(
        const std::string& config_file
    );
    
    static void writeConfigFile(
        const std::unordered_map<std::string, std::string>& config_map,
        const std::string& config_file
    );
};

// Command-line interface for the arbitrage analyzer
class ArbitrageCLI {
public:
    // Parse command-line arguments
    static ArbitrageAnalyzer::AnalysisConfig parseCommandLine(int argc, char* argv[]);
    
    // Print usage information
    static void printUsage();
    
    // Print analysis results summary
    static void printResultsSummary(const ArbitrageAnalyzer::AnalysisMetrics& metrics);
    
    // Interactive configuration
    static ArbitrageAnalyzer::AnalysisConfig interactiveConfig();

private:
    // Command-line option parsing
    static void parseOption(
        const std::string& option,
        const std::string& value,
        ArbitrageAnalyzer::AnalysisConfig& config
    );
};
