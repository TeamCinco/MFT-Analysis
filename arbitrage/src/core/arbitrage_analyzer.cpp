#include "arbitrage_analyzer.h"
#include <iostream>
#include <chrono>

// Static member initialization
ArbitrageAnalyzer::AnalysisMetrics ArbitrageAnalyzer::last_metrics_;
ArbitrageAnalyzer::ProgressCallback ArbitrageAnalyzer::progress_callback_;
std::chrono::high_resolution_clock::time_point ArbitrageAnalyzer::analysis_start_time_;
std::mutex ArbitrageAnalyzer::progress_mutex_;
std::atomic<size_t> ArbitrageAnalyzer::pairs_completed_{0};
std::atomic<size_t> ArbitrageAnalyzer::total_pairs_{0};

// AnalysisCache static member initialization
std::unordered_map<std::string, CointegrationResult> AnalysisCache::cointegration_cache_;
std::unordered_map<std::string, CorrelationResult> AnalysisCache::correlation_cache_;
AnalysisCache::CacheStats AnalysisCache::cache_stats_;
std::mutex AnalysisCache::cache_mutex_;

bool ArbitrageAnalyzer::runFullAnalysis() {
    return runFullAnalysis(ConfigManager::createDefaultConfig());
}

bool ArbitrageAnalyzer::runFullAnalysis(const AnalysisConfig& config) {
    analysis_start_time_ = std::chrono::high_resolution_clock::now();
    
    try {
        // Reset metrics
        last_metrics_ = AnalysisMetrics{};
        
        reportProgress("Loading Data", 0.0);
        
        // Load stock data
        auto stocks = loadStockData(config);
        if (stocks.empty()) {
            std::cerr << "No stock data loaded" << std::endl;
            return false;
        }
        
        last_metrics_.stocks_loaded = stocks.size();
        reportProgress("Loading Data", 100.0);
        
        reportProgress("Analyzing Cointegration", 0.0);
        
        // Analyze cointegration
        auto cointegration_results = analyzeCointegration(stocks, config);
        last_metrics_.cointegrated_pairs_found = cointegration_results.size();
        
        reportProgress("Analyzing Correlation", 0.0);
        
        // Analyze correlation
        auto correlation_results = analyzeCorrelation(stocks, config);
        last_metrics_.high_correlation_pairs_found = correlation_results.size();
        
        reportProgress("Generating Opportunities", 0.0);
        
        // Generate opportunities
        auto opportunities = generateOpportunities(cointegration_results, correlation_results, config);
        last_metrics_.arbitrage_opportunities_found = opportunities.size();
        
        reportProgress("Exporting Results", 0.0);
        
        // Export results
        bool export_success = exportResults(cointegration_results, correlation_results, opportunities, config);
        last_metrics_.export_successful = export_success;
        
        reportProgress("Complete", 100.0);
        
        // Calculate final metrics
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - analysis_start_time_);
        last_metrics_.total_time_seconds = duration.count() / 1000.0;
        
        // Set timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        last_metrics_.analysis_timestamp = std::ctime(&time_t);
        
        return export_success;
        
    } catch (const std::exception& e) {
        std::cerr << "Analysis failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::unique_ptr<StockData>> ArbitrageAnalyzer::loadStockData(const AnalysisConfig& config) {
    return FastCSVLoader::loadAllStocks(config.input_data_directory, config.portfolio_constraints);
}

std::vector<CointegrationResult> ArbitrageAnalyzer::analyzeCointegration(
    const std::vector<std::unique_ptr<StockData>>& stocks,
    const AnalysisConfig& config) {
    
    std::vector<CointegrationResult> results;
    
    if (stocks.size() < 2) {
        return results;
    }
    
    // Generate all possible pairs for analysis
    std::vector<std::pair<const StockData*, const StockData*>> stock_pairs;
    
    size_t max_pairs = config.max_pairs_to_analyze > 0 ? 
                      static_cast<size_t>(config.max_pairs_to_analyze) : 
                      stocks.size() * (stocks.size() - 1) / 2;
    
    size_t pairs_generated = 0;
    
    for (size_t i = 0; i < stocks.size() && pairs_generated < max_pairs; ++i) {
        for (size_t j = i + 1; j < stocks.size() && pairs_generated < max_pairs; ++j) {
            if (stocks[i] && stocks[j]) {
                // Check if pair meets basic criteria
                if (isValidPair(*stocks[i], *stocks[j], config)) {
                    stock_pairs.emplace_back(stocks[i].get(), stocks[j].get());
                    pairs_generated++;
                }
            }
        }
    }
    
    total_pairs_ = stock_pairs.size();
    pairs_completed_ = 0;
    
    reportProgress("Analyzing Cointegration", 0.0);
    
    // Analyze pairs in parallel using enhanced cointegration analyzer
    if (config.num_threads > 1) {
        results = analyzeCointegrationParallel(stocks, config);
    } else {
        // Sequential analysis
        results.reserve(stock_pairs.size());
        
        for (size_t i = 0; i < stock_pairs.size(); ++i) {
            auto result = SIMDCointegrationAnalyzer::analyzeCointegration_SIMD(
                *stock_pairs[i].first, *stock_pairs[i].second);
            
            // Only keep cointegrated pairs that meet our criteria
            if (result.is_cointegrated && 
                result.p_value <= config.max_cointegration_pvalue &&
                result.half_life > 0 && result.half_life < 100) {
                results.push_back(result);
            }
            
            pairs_completed_++;
            if (i % 100 == 0) {
                double progress = static_cast<double>(i) / stock_pairs.size() * 100.0;
                reportProgress("Analyzing Cointegration", progress);
            }
        }
    }
    
    reportProgress("Analyzing Cointegration", 100.0);
    
    // Sort results by statistical significance (p-value ascending)
    std::sort(results.begin(), results.end(), 
              [](const CointegrationResult& a, const CointegrationResult& b) {
                  return a.p_value < b.p_value;
              });
    
    return results;
}

std::vector<CorrelationResult> ArbitrageAnalyzer::analyzeCorrelation(
    const std::vector<std::unique_ptr<StockData>>& stocks,
    const AnalysisConfig& config) {
    
    // Placeholder implementation
    std::vector<CorrelationResult> results;
    
    // For now, just create some dummy results to test the system
    if (stocks.size() >= 2) {
        CorrelationResult dummy_result;
        dummy_result.stock1 = stocks[0]->symbol;
        dummy_result.stock2 = stocks[1]->symbol;
        dummy_result.pearson_correlation = 0.85;
        dummy_result.spearman_correlation = 0.82;
        dummy_result.correlation_stability = 0.9;
        dummy_result.correlation_grade = "A";
        
        results.push_back(dummy_result);
    }
    
    return results;
}

std::vector<ArbitrageOpportunity> ArbitrageAnalyzer::generateOpportunities(
    const std::vector<CointegrationResult>& cointegration_results,
    const std::vector<CorrelationResult>& correlation_results,
    const AnalysisConfig& config) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // Simple matching of cointegration and correlation results
    for (const auto& coint : cointegration_results) {
        for (const auto& corr : correlation_results) {
            if (coint.stock1 == corr.stock1 && coint.stock2 == corr.stock2) {
                ArbitrageOpportunity opp;
                opp.stock1 = coint.stock1;
                opp.stock2 = coint.stock2;
                opp.cointegration_score = coint.is_cointegrated ? 90.0 : 50.0;
                opp.correlation_score = corr.pearson_correlation * 100.0;
                opp.combined_score = (opp.cointegration_score + opp.correlation_score) / 2.0;
                opp.profit_potential = 0.05; // 5% expected return
                opp.opportunity_grade = "A";
                
                opportunities.push_back(opp);
            }
        }
    }
    
    return opportunities;
}

bool ArbitrageAnalyzer::exportResults(
    const std::vector<CointegrationResult>& cointegration_results,
    const std::vector<CorrelationResult>& correlation_results,
    const std::vector<ArbitrageOpportunity>& opportunities,
    const AnalysisConfig& config) {
    
    // For now, just export to CSV
    return CSVExporter::exportAllToCSV(
        cointegration_results,
        correlation_results,
        opportunities,
        config.output_directory
    );
}

void ArbitrageAnalyzer::reportProgress(const std::string& stage, double progress) {
    if (progress_callback_) {
        progress_callback_(stage, progress);
    }
}

// Check if a pair of stocks is valid for analysis
bool ArbitrageAnalyzer::isValidPair(
    const StockData& stock1,
    const StockData& stock2,
    const AnalysisConfig& config) {
    
    // Check minimum data points
    if (stock1.size() < config.min_data_points || stock2.size() < config.min_data_points) {
        return false;
    }
    
    // Check if data sizes match
    if (stock1.size() != stock2.size()) {
        return false;
    }
    
    // Check sector requirements
    if (config.require_same_sector && stock1.sector != stock2.sector) {
        return false;
    }
    
    // Check if symbols are in excluded list
    for (const auto& excluded : config.excluded_symbols) {
        if (stock1.symbol == excluded || stock2.symbol == excluded) {
            return false;
        }
    }
    
    // Check price constraints
    if (!stock1.close.empty() && !stock2.close.empty()) {
        double price1 = stock1.close.back();
        double price2 = stock2.close.back();
        
        if (price1 < config.portfolio_constraints.min_stock_price || 
            price1 > config.portfolio_constraints.max_stock_price ||
            price2 < config.portfolio_constraints.min_stock_price || 
            price2 > config.portfolio_constraints.max_stock_price) {
            return false;
        }
    }
    
    return true;
}

// Parallel cointegration analysis implementation
std::vector<CointegrationResult> ArbitrageAnalyzer::analyzeCointegrationParallel(
    const std::vector<std::unique_ptr<StockData>>& stocks,
    const AnalysisConfig& config) {
    
    // For now, fall back to sequential analysis
    // In a full implementation, this would use thread pools
    std::vector<CointegrationResult> results;
    
    if (stocks.size() < 2) {
        return results;
    }
    
    // Generate pairs
    std::vector<std::pair<const StockData*, const StockData*>> stock_pairs;
    
    size_t max_pairs = config.max_pairs_to_analyze > 0 ? 
                      static_cast<size_t>(config.max_pairs_to_analyze) : 
                      stocks.size() * (stocks.size() - 1) / 2;
    
    size_t pairs_generated = 0;
    
    for (size_t i = 0; i < stocks.size() && pairs_generated < max_pairs; ++i) {
        for (size_t j = i + 1; j < stocks.size() && pairs_generated < max_pairs; ++j) {
            if (stocks[i] && stocks[j]) {
                if (isValidPair(*stocks[i], *stocks[j], config)) {
                    stock_pairs.emplace_back(stocks[i].get(), stocks[j].get());
                    pairs_generated++;
                }
            }
        }
    }
    
    // Analyze pairs sequentially for now
    results.reserve(stock_pairs.size());
    
    for (size_t i = 0; i < stock_pairs.size(); ++i) {
        auto result = SIMDCointegrationAnalyzer::analyzeCointegration_SIMD(
            *stock_pairs[i].first, *stock_pairs[i].second);
        
        if (result.is_cointegrated && 
            result.p_value <= config.max_cointegration_pvalue &&
            result.half_life > 0 && result.half_life < 100) {
            results.push_back(result);
        }
        
        pairs_completed_++;
        if (i % 100 == 0) {
            double progress = static_cast<double>(i) / stock_pairs.size() * 100.0;
            reportProgress("Analyzing Cointegration", progress);
        }
    }
    
    return results;
}

// Configuration management implementation
ArbitrageAnalyzer::AnalysisConfig ConfigManager::createDefaultConfig() {
    ArbitrageAnalyzer::AnalysisConfig config;
    // Default values are already set in the struct definition
    return config;
}

bool ConfigManager::validateConfig(const ArbitrageAnalyzer::AnalysisConfig& config) {
    // Basic validation
    if (config.min_correlation_threshold < 0.0 || config.min_correlation_threshold > 1.0) {
        return false;
    }
    
    if (config.max_cointegration_pvalue < 0.0 || config.max_cointegration_pvalue > 1.0) {
        return false;
    }
    
    if (config.portfolio_constraints.max_capital <= 0.0) {
        return false;
    }
    
    return true;
}

// CLI implementation
ArbitrageAnalyzer::AnalysisConfig ArbitrageCLI::parseCommandLine(int argc, char* argv[]) {
    auto config = ConfigManager::createDefaultConfig();
    
    // Simple command line parsing
    for (int i = 1; i < argc; i++) {
        std::string option = argv[i];
        
        // Skip non-option arguments like --benchmark
        if (option == "--benchmark" || option == "--interactive" || option == "--help" || option == "-h") {
            continue;
        }
        
        // For options that require values
        if (i + 1 < argc && option.substr(0, 2) == "--") {
            std::string value = argv[i + 1];
            parseOption(option, value, config);
            i++; // Skip the value argument
        }
    }
    
    return config;
}

void ArbitrageCLI::parseOption(
    const std::string& option,
    const std::string& value,
    ArbitrageAnalyzer::AnalysisConfig& config) {
    
    if (option == "--input-dir") {
        config.input_data_directory = value;
    } else if (option == "--output-dir") {
        config.output_directory = value;
    } else if (option == "--min-correlation") {
        config.min_correlation_threshold = std::stod(value);
    }
    // Add more options as needed
}

void ArbitrageCLI::printUsage() {
    std::cout << "MFT Statistical Arbitrage Analyzer\n";
    std::cout << "Usage: arbitrage_analyzer [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --input-dir PATH     Input data directory\n";
    std::cout << "  --output-dir PATH    Output directory\n";
    std::cout << "  --min-correlation N  Minimum correlation threshold\n";
    std::cout << "  --benchmark          Run performance benchmark\n";
    std::cout << "  --interactive        Interactive configuration\n";
    std::cout << "  --help               Show this help\n";
}

ArbitrageAnalyzer::AnalysisConfig ArbitrageCLI::interactiveConfig() {
    // For now, just return default config
    // In a full implementation, this would prompt the user for input
    return ConfigManager::createDefaultConfig();
}
