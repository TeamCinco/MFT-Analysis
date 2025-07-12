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
    
    // Placeholder implementation
    std::vector<CointegrationResult> results;
    
    // For now, just create some dummy results to test the system
    if (stocks.size() >= 2) {
        CointegrationResult dummy_result;
        dummy_result.stock1 = stocks[0]->symbol;
        dummy_result.stock2 = stocks[1]->symbol;
        dummy_result.adf_statistic = -3.5;
        dummy_result.p_value = 0.01;
        dummy_result.hedge_ratio = 1.2;
        dummy_result.is_cointegrated = true;
        dummy_result.cointegration_grade = "A";
        
        results.push_back(dummy_result);
    }
    
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
