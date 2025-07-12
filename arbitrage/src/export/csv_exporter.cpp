#include "excel_exporter.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

// Static member initialization
ExcelExporter::ExcelFormatConfig ExcelExporter::format_config_;
std::chrono::high_resolution_clock::time_point ExportPerformanceTracker::start_time_;
ExportPerformanceTracker::ExportMetrics ExportPerformanceTracker::last_metrics_;

bool CSVExporter::exportCointegrationCSV(
    const std::vector<CointegrationResult>& results,
    const std::string& output_path) {
    
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    writeCointegrationHeader(file);
    
    // Write data
    for (const auto& result : results) {
        file << escapeCSVField(result.stock1) << ","
             << escapeCSVField(result.stock2) << ","
             << std::fixed << std::setprecision(6) << result.adf_statistic << ","
             << std::fixed << std::setprecision(6) << result.p_value << ","
             << std::fixed << std::setprecision(6) << result.critical_value_1pct << ","
             << std::fixed << std::setprecision(6) << result.critical_value_5pct << ","
             << std::fixed << std::setprecision(6) << result.critical_value_10pct << ","
             << std::fixed << std::setprecision(2) << result.half_life << ","
             << std::fixed << std::setprecision(6) << result.hedge_ratio << ","
             << std::fixed << std::setprecision(6) << result.spread_mean << ","
             << std::fixed << std::setprecision(6) << result.spread_std << ","
             << std::fixed << std::setprecision(6) << result.max_spread << ","
             << std::fixed << std::setprecision(6) << result.min_spread << ","
             << std::fixed << std::setprecision(6) << result.current_spread << ","
             << std::fixed << std::setprecision(6) << result.z_score << ","
             << escapeCSVField(result.cointegration_grade) << ","
             << (result.is_cointegrated ? "TRUE" : "FALSE") << ","
             << std::fixed << std::setprecision(6) << result.entry_threshold << ","
             << std::fixed << std::setprecision(6) << result.exit_threshold << ","
             << std::fixed << std::setprecision(6) << result.expected_return << ","
             << std::fixed << std::setprecision(6) << result.sharpe_ratio << ","
             << result.num_trades_historical << ","
             << std::fixed << std::setprecision(6) << result.win_rate << "\n";
    }
    
    return true;
}

bool CSVExporter::exportCorrelationCSV(
    const std::vector<CorrelationResult>& results,
    const std::string& output_path) {
    
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    writeCorrelationHeader(file);
    
    // Write data
    for (const auto& result : results) {
        file << escapeCSVField(result.stock1) << ","
             << escapeCSVField(result.stock2) << ","
             << std::fixed << std::setprecision(6) << result.pearson_correlation << ","
             << std::fixed << std::setprecision(6) << result.spearman_correlation << ","
             << std::fixed << std::setprecision(6) << result.kendall_tau << ","
             << std::fixed << std::setprecision(6) << result.rolling_correlation_30d << ","
             << std::fixed << std::setprecision(6) << result.rolling_correlation_60d << ","
             << std::fixed << std::setprecision(6) << result.correlation_stability << ","
             << std::fixed << std::setprecision(0) << result.correlation_breakdown_count << ","
             << std::fixed << std::setprecision(6) << result.min_correlation << ","
             << std::fixed << std::setprecision(6) << result.max_correlation << ","
             << escapeCSVField(result.correlation_grade) << ","
             << escapeCSVField(result.sector1) << ","
             << escapeCSVField(result.sector2) << ","
             << (result.same_sector ? "TRUE" : "FALSE") << ","
             << std::fixed << std::setprecision(2) << result.price1 << ","
             << std::fixed << std::setprecision(2) << result.price2 << ","
             << (result.affordable_pair ? "TRUE" : "FALSE") << "\n";
    }
    
    return true;
}

bool CSVExporter::exportOpportunitiesCSV(
    const std::vector<ArbitrageOpportunity>& opportunities,
    const std::string& output_path) {
    
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    writeOpportunitiesHeader(file);
    
    // Write data
    for (const auto& opp : opportunities) {
        file << escapeCSVField(opp.stock1) << ","
             << escapeCSVField(opp.stock2) << ","
             << std::fixed << std::setprecision(2) << opp.cointegration_score << ","
             << std::fixed << std::setprecision(2) << opp.correlation_score << ","
             << std::fixed << std::setprecision(2) << opp.combined_score << ","
             << std::fixed << std::setprecision(4) << opp.profit_potential << ","
             << std::fixed << std::setprecision(2) << opp.risk_score << ","
             << std::fixed << std::setprecision(2) << opp.entry_z_score << ","
             << std::fixed << std::setprecision(2) << opp.exit_z_score << ","
             << std::fixed << std::setprecision(2) << opp.stop_loss_z_score << ","
             << std::fixed << std::setprecision(0) << opp.position_size_stock1 << ","
             << std::fixed << std::setprecision(6) << opp.position_size_stock2 << ","
             << std::fixed << std::setprecision(2) << opp.cash_required << ","
             << std::fixed << std::setprecision(4) << opp.expected_sharpe_ratio << ","
             << std::fixed << std::setprecision(4) << opp.max_drawdown << ","
             << std::fixed << std::setprecision(0) << opp.expected_trades_per_year << ","
             << escapeCSVField(opp.market_regime) << ","
             << escapeCSVField(opp.volatility_regime) << ","
             << escapeCSVField(opp.opportunity_grade) << "\n";
    }
    
    return true;
}

bool CSVExporter::exportAllToCSV(
    const std::vector<CointegrationResult>& cointegration_results,
    const std::vector<CorrelationResult>& correlation_results,
    const std::vector<ArbitrageOpportunity>& opportunities,
    const std::string& output_directory) {
    
    ExportPerformanceTracker::startExportTiming();
    
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(output_directory);
    
    bool success = true;
    size_t total_records = 0;
    
    // Export cointegration results
    std::string coint_path = output_directory + "/cointegration_results.csv";
    if (!exportCointegrationCSV(cointegration_results, coint_path)) {
        success = false;
    } else {
        total_records += cointegration_results.size();
    }
    
    // Export correlation results
    std::string corr_path = output_directory + "/correlation_results.csv";
    if (!exportCorrelationCSV(correlation_results, corr_path)) {
        success = false;
    } else {
        total_records += correlation_results.size();
    }
    
    // Export opportunities
    std::string opp_path = output_directory + "/arbitrage_opportunities.csv";
    if (!exportOpportunitiesCSV(opportunities, opp_path)) {
        success = false;
    } else {
        total_records += opportunities.size();
    }
    
    // Calculate total file size
    size_t total_size = 0;
    try {
        if (std::filesystem::exists(coint_path)) {
            total_size += std::filesystem::file_size(coint_path);
        }
        if (std::filesystem::exists(corr_path)) {
            total_size += std::filesystem::file_size(corr_path);
        }
        if (std::filesystem::exists(opp_path)) {
            total_size += std::filesystem::file_size(opp_path);
        }
    } catch (const std::exception&) {
        // Ignore file size calculation errors
    }
    
    ExportPerformanceTracker::endExportTiming(total_records, total_size, "CSV", success);
    
    return success;
}

std::string CSVExporter::escapeCSVField(const std::string& field) {
    if (field.find(',') != std::string::npos || 
        field.find('"') != std::string::npos || 
        field.find('\n') != std::string::npos) {
        
        std::string escaped = "\"";
        for (char c : field) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += "\"";
        return escaped;
    }
    return field;
}

void CSVExporter::writeCointegrationHeader(std::ofstream& file) {
    file << "Stock1,Stock2,ADF_Statistic,P_Value,Critical_1pct,Critical_5pct,Critical_10pct,"
         << "Half_Life_Days,Hedge_Ratio,Spread_Mean,Spread_StdDev,Max_Spread,Min_Spread,"
         << "Current_Spread,Z_Score,Grade,Is_Cointegrated,Entry_Threshold,Exit_Threshold,"
         << "Expected_Return,Sharpe_Ratio,Historical_Trades,Win_Rate\n";
}

void CSVExporter::writeCorrelationHeader(std::ofstream& file) {
    file << "Stock1,Stock2,Pearson_Correlation,Spearman_Correlation,Kendall_Tau,"
         << "Rolling_30d,Rolling_60d,Stability,Breakdown_Count,Min_Correlation,"
         << "Max_Correlation,Grade,Sector1,Sector2,Same_Sector,Price1,Price2,Affordable\n";
}

void CSVExporter::writeOpportunitiesHeader(std::ofstream& file) {
    file << "Stock1,Stock2,Cointegration_Score,Correlation_Score,Combined_Score,"
         << "Profit_Potential,Risk_Score,Entry_Z_Score,Exit_Z_Score,Stop_Loss_Z_Score,"
         << "Position_Size_Stock1,Position_Size_Stock2,Cash_Required,Expected_Sharpe,"
         << "Max_Drawdown,Expected_Trades_Per_Year,Market_Regime,Volatility_Regime,Grade\n";
}

// Export performance tracker implementation
void ExportPerformanceTracker::startExportTiming() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

void ExportPerformanceTracker::endExportTiming(
    size_t records, size_t file_size, const std::string& format, bool success) {
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
    
    last_metrics_.export_time_seconds = duration.count() / 1000.0;
    last_metrics_.records_exported = records;
    last_metrics_.file_size_bytes = file_size;
    last_metrics_.export_format = format;
    last_metrics_.export_successful = success;
    
    if (last_metrics_.export_time_seconds > 0.0) {
        last_metrics_.records_per_second = records / last_metrics_.export_time_seconds;
    }
}

// Placeholder implementations for Excel and JSON exporters
bool ExcelExporter::exportArbitrageReport(
    const std::vector<CointegrationResult>& cointegration_results,
    const std::vector<CorrelationResult>& correlation_results,
    const std::vector<ArbitrageOpportunity>& opportunities,
    const std::string& output_path) {
    
    // For now, fall back to CSV export
    std::string csv_dir = output_path + "_csv";
    return CSVExporter::exportAllToCSV(cointegration_results, correlation_results, opportunities, csv_dir);
}

bool JSONExporter::exportToJSON(
    const std::vector<CointegrationResult>& cointegration_results,
    const std::vector<CorrelationResult>& correlation_results,
    const std::vector<ArbitrageOpportunity>& opportunities,
    const std::string& output_path) {
    
    // Placeholder implementation
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"cointegration_results\": " << cointegrationToJSON(cointegration_results) << ",\n";
    file << "  \"correlation_results\": " << correlationToJSON(correlation_results) << ",\n";
    file << "  \"arbitrage_opportunities\": " << opportunitiesToJSON(opportunities) << "\n";
    file << "}\n";
    
    return true;
}

std::string JSONExporter::cointegrationToJSON(const std::vector<CointegrationResult>& results) {
    std::string json = "[\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        json += "    {\n";
        json += "      \"stock1\": \"" + escapeJSON(r.stock1) + "\",\n";
        json += "      \"stock2\": \"" + escapeJSON(r.stock2) + "\",\n";
        json += "      \"adf_statistic\": " + doubleToString(r.adf_statistic) + ",\n";
        json += "      \"p_value\": " + doubleToString(r.p_value) + ",\n";
        json += "      \"hedge_ratio\": " + doubleToString(r.hedge_ratio) + ",\n";
        json += "      \"is_cointegrated\": ";
        json += (r.is_cointegrated ? "true" : "false");
        json += ",\n";
        json += "      \"grade\": \"" + escapeJSON(r.cointegration_grade) + "\"\n";
        json += "    }";
        if (i < results.size() - 1) json += ",";
        json += "\n";
    }
    json += "  ]";
    return json;
}

std::string JSONExporter::correlationToJSON(const std::vector<CorrelationResult>& results) {
    return "[]"; // Placeholder
}

std::string JSONExporter::opportunitiesToJSON(const std::vector<ArbitrageOpportunity>& opportunities) {
    return "[]"; // Placeholder
}

std::string JSONExporter::escapeJSON(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string JSONExporter::doubleToString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
