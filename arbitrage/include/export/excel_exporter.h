#pragma once

#include "../core/stock_data.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <chrono>
#include <sstream>

// Excel export functionality for arbitrage results
class ExcelExporter {
public:
    // Export comprehensive arbitrage analysis to Excel
    static bool exportArbitrageReport(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path
    );
    
    // Export individual analysis sheets
    static bool exportCointegrationSheet(
        const std::vector<CointegrationResult>& results,
        const std::string& output_path,
        const std::string& sheet_name = "Cointegration_Analysis"
    );
    
    static bool exportCorrelationSheet(
        const std::vector<CorrelationResult>& results,
        const std::string& output_path,
        const std::string& sheet_name = "Correlation_Analysis"
    );
    
    static bool exportOpportunitiesSheet(
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path,
        const std::string& sheet_name = "Arbitrage_Opportunities"
    );
    
    // Export performance metrics and summary statistics
    static bool exportSummarySheet(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path,
        const std::string& sheet_name = "Summary_Statistics"
    );
    
    // Export sector analysis
    static bool exportSectorAnalysisSheet(
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path,
        const std::string& sheet_name = "Sector_Analysis"
    );
    
    // Configuration for Excel formatting
    struct ExcelFormatConfig {
        bool use_conditional_formatting = true;
        bool include_charts = true;
        bool freeze_header_row = true;
        bool auto_filter = true;
        bool bold_headers = true;
        std::string header_color = "#4472C4";
        std::string grade_a_color = "#70AD47";
        std::string grade_b_color = "#FFC000";
        std::string grade_c_color = "#FF9900";
        std::string grade_d_color = "#C5504B";
    };
    
    static void setFormatConfig(const ExcelFormatConfig& config) { format_config_ = config; }

private:
    // CSV export as fallback (if Excel library not available)
    static bool exportToCSV(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_directory
    );
    
    // Helper functions for Excel formatting
    static void formatHeader(void* worksheet, int row, int start_col, int end_col);
    static void applyConditionalFormatting(void* worksheet, const std::string& range, const std::string& column_type);
    static void addChart(void* workbook, void* worksheet, const std::string& chart_type, const std::string& data_range);
    
    // Data validation and sorting
    static std::vector<CointegrationResult> sortCointegrationResults(
        const std::vector<CointegrationResult>& results
    );
    
    static std::vector<CorrelationResult> sortCorrelationResults(
        const std::vector<CorrelationResult>& results
    );
    
    static std::vector<ArbitrageOpportunity> sortOpportunities(
        const std::vector<ArbitrageOpportunity>& opportunities
    );
    
    // Summary statistics calculation
    struct SummaryStats {
        int total_pairs_analyzed;
        int cointegrated_pairs;
        int high_correlation_pairs;
        int grade_a_opportunities;
        int grade_b_opportunities;
        int grade_c_opportunities;
        int grade_d_opportunities;
        double avg_cointegration_score;
        double avg_correlation_score;
        double avg_profit_potential;
        double total_capital_required;
        std::string best_sector_pair;
        std::string most_profitable_pair;
    };
    
    static SummaryStats calculateSummaryStats(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities
    );
    
    // Sector analysis
    struct SectorStats {
        std::string sector_name;
        int num_pairs;
        double avg_correlation;
        double avg_cointegration_score;
        int num_opportunities;
        double avg_profit_potential;
    };
    
    static std::vector<SectorStats> calculateSectorStats(
        const std::vector<ArbitrageOpportunity>& opportunities
    );
    
    static ExcelFormatConfig format_config_;
};

// CSV exporter for simple text output
class CSVExporter {
public:
    // Export cointegration results to CSV
    static bool exportCointegrationCSV(
        const std::vector<CointegrationResult>& results,
        const std::string& output_path
    );
    
    // Export correlation results to CSV
    static bool exportCorrelationCSV(
        const std::vector<CorrelationResult>& results,
        const std::string& output_path
    );
    
    // Export arbitrage opportunities to CSV
    static bool exportOpportunitiesCSV(
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path
    );
    
    // Export all results to separate CSV files
    static bool exportAllToCSV(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_directory
    );

private:
    // Helper function to escape CSV fields
    static std::string escapeCSVField(const std::string& field);
    
    // Write CSV header
    static void writeCointegrationHeader(std::ofstream& file);
    static void writeCorrelationHeader(std::ofstream& file);
    static void writeOpportunitiesHeader(std::ofstream& file);
};

// JSON exporter for programmatic access
class JSONExporter {
public:
    // Export results to JSON format
    static bool exportToJSON(
        const std::vector<CointegrationResult>& cointegration_results,
        const std::vector<CorrelationResult>& correlation_results,
        const std::vector<ArbitrageOpportunity>& opportunities,
        const std::string& output_path
    );
    
    // Export individual result types
    static std::string cointegrationToJSON(const std::vector<CointegrationResult>& results);
    static std::string correlationToJSON(const std::vector<CorrelationResult>& results);
    static std::string opportunitiesToJSON(const std::vector<ArbitrageOpportunity>& opportunities);

private:
    // JSON formatting helpers
    static std::string escapeJSON(const std::string& str);
    static std::string doubleToString(double value, int precision = 6);
};

// Performance metrics for export operations
class ExportPerformanceTracker {
public:
    struct ExportMetrics {
        double export_time_seconds = 0.0;
        size_t records_exported = 0;
        size_t file_size_bytes = 0;
        double records_per_second = 0.0;
        std::string export_format; // "Excel", "CSV", "JSON"
        bool export_successful = false;
    };
    
    static void startExportTiming();
    static void endExportTiming(size_t records, size_t file_size, const std::string& format, bool success);
    static ExportMetrics getLastExportMetrics() { return last_metrics_; }

private:
    static std::chrono::high_resolution_clock::time_point start_time_;
    static ExportMetrics last_metrics_;
};
