#pragma once

#include "core/FeatureRegistry.h"
#include "core/FeatureExtractor.h"
#include "core/ChartFactory.h"
#include "rendering/ModularChartRenderer.h"
#include "../../feature_engineering/include/ohlcv_data.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Visualization {

// Data loading and management
class DataManager {
public:
    // Load data from CSV files (existing format)
    static std::vector<FlexibleStockData> loadFromCSV(const std::string& csv_path);
    
    // Convert from FeatureSet to FlexibleStockData
    static std::vector<FlexibleStockData> convertFromFeatureSet(
        const std::string& symbol,
        const OHLCVData& ohlcv_data,
        const FeatureSet& feature_set);
    
    // Batch load multiple symbols
    static std::unordered_map<std::string, std::vector<FlexibleStockData>> loadMultipleSymbols(
        const std::vector<std::string>& csv_paths);
    
    // Data validation and cleaning
    static void validateData(std::vector<FlexibleStockData>& data);
    static void cleanData(std::vector<FlexibleStockData>& data, bool remove_outliers = false);
    
private:
    static std::string extractSymbolFromPath(const std::string& csv_path);
};

// Main visualization manager class
class VisualizationManager {
public:
    VisualizationManager();
    ~VisualizationManager() = default;
    
    // Initialization
    void initialize();
    void initializeFeatureRegistry();
    
    // Data management
    bool loadData(const std::string& csv_path);
    bool loadData(const std::string& symbol, const OHLCVData& ohlcv_data, const FeatureSet& feature_set);
    bool loadMultipleDataSources(const std::vector<std::string>& csv_paths);
    
    void setCurrentSymbol(const std::string& symbol);
    std::string getCurrentSymbol() const { return current_symbol_; }
    
    // Rendering methods
    void renderMainDashboard();
    void renderSymbolSelector();
    void renderFeatureDashboard();
    void renderComparisonDashboard();
    void renderStatisticsDashboard();
    
    // Feature management
    void addFeature(const std::string& name, const std::string& display_name, 
                   FeatureCategory category, ChartType chart_type = ChartType::LINE);
    
    std::vector<std::string> getAvailableSymbols() const;
    std::vector<std::string> getAvailableFeatures() const;
    std::vector<FeatureCategory> getAvailableCategories() const;
    
    // Configuration
    void setDashboardLayout(const DashboardLayout& layout);
    void setChartFilter(const ChartFilter& filter);
    
    DashboardLayout& getDashboardLayout() { return renderer_.getDashboardLayout(); }
    ChartFilter& getChartFilter() { return renderer_.getChartFilter(); }
    
    // Utility methods
    bool hasDataForSymbol(const std::string& symbol) const;
    size_t getDataPointCount(const std::string& symbol = "") const;
    
    // Export functionality
    void exportChartAsImage(const std::string& feature_name, const std::string& output_path);
    void exportDashboardAsImages(const std::string& output_directory);
    
    // Demo and example data
    void loadDemoData();
    void createExampleDashboard();

private:
    // Core components
    ModularChartRenderer renderer_;
    
    // Data storage
    std::unordered_map<std::string, std::vector<FlexibleStockData>> symbol_data_;
    std::string current_symbol_;
    
    // UI state
    bool show_symbol_selector_;
    bool show_feature_dashboard_;
    bool show_comparison_dashboard_;
    bool show_statistics_dashboard_;
    std::vector<std::string> selected_symbols_for_comparison_;
    
    // Helper methods
    const std::vector<FlexibleStockData>& getCurrentData() const;
    std::vector<FlexibleStockData>& getCurrentData();
    
    void renderMainMenuBar();
    void renderStatusBar();
    void renderDataInfo();
    
    // UI helpers
    void renderSymbolCombo();
    void renderFeatureSelectionUI();
    void renderLayoutSelectionUI();
    
    // Error handling
    void showErrorMessage(const std::string& message);
    void showInfoMessage(const std::string& message);
    
    std::string last_error_message_;
    std::string last_info_message_;
    bool show_error_popup_;
    bool show_info_popup_;
};

// Convenience functions for easy usage
namespace EasyViz {
    // Simple one-liner functions for common use cases
    
    // Load and visualize a single CSV file
    void visualizeCSV(const std::string& csv_path);
    
    // Load and visualize from FeatureSet
    void visualizeFeatureSet(const std::string& symbol, 
                           const OHLCVData& ohlcv_data, 
                           const FeatureSet& feature_set);
    
    // Create a quick comparison chart
    void compareFeatures(const std::vector<std::string>& feature_names,
                        const std::string& csv_path);
    
    // Create a category overview
    void visualizeCategory(FeatureCategory category, const std::string& csv_path);
    
    // Create a statistical summary
    void showStatistics(const std::string& csv_path);
}

} // namespace Visualization
