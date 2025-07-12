#pragma once

#include "core/FeatureRegistry.h"
#include "core/FeatureExtractor.h"
#include "core/ChartFactory.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Visualization {

// Dashboard layout configuration
struct DashboardLayout {
    enum class LayoutType {
        GRID,
        TABS,
        ACCORDION,
        CUSTOM
    };
    
    LayoutType type;
    int columns;
    int rows;
    bool auto_resize;
    float chart_spacing;
    
    DashboardLayout(LayoutType t = LayoutType::GRID, int cols = 2, int rows = 3)
        : type(t), columns(cols), rows(rows), auto_resize(true), chart_spacing(10.0f) {}
};

// Chart selection and filtering
struct ChartFilter {
    std::vector<FeatureCategory> enabled_categories;
    std::vector<std::string> enabled_features;
    std::vector<std::string> disabled_features;
    bool show_price_charts;
    bool show_volume_charts;
    bool show_technical_indicators;
    bool show_statistical_features;
    
    ChartFilter() 
        : show_price_charts(true), show_volume_charts(true),
          show_technical_indicators(true), show_statistical_features(true) {}
    
    bool isFeatureEnabled(const std::string& feature_name) const;
    bool isCategoryEnabled(FeatureCategory category) const;
};

class ModularChartRenderer {
public:
    ModularChartRenderer();
    ~ModularChartRenderer() = default;
    
    // Main rendering methods
    void renderDashboard(const std::vector<FlexibleStockData>& data, 
                        const std::string& symbol = "");
    
    void renderCategoryDashboard(const std::vector<FlexibleStockData>& data,
                               const std::string& symbol = "");
    
    void renderCustomDashboard(const std::vector<FlexibleStockData>& data,
                             const std::vector<std::string>& selected_features,
                             const std::string& symbol = "");
    
    // Individual chart rendering
    void renderFeatureChart(const std::string& feature_name,
                          const std::vector<FlexibleStockData>& data,
                          const std::string& symbol = "");
    
    void renderCategoryChart(FeatureCategory category,
                           const std::vector<FlexibleStockData>& data,
                           const std::string& symbol = "");
    
    void renderComparisonChart(const std::vector<std::string>& feature_names,
                             const std::vector<FlexibleStockData>& data,
                             const std::string& title = "",
                             const std::string& symbol = "");
    
    // Special chart types
    void renderPriceVolumeOverview(const std::vector<FlexibleStockData>& data,
                                 const std::string& symbol = "");
    
    void renderStatisticalSummary(const std::vector<FlexibleStockData>& data,
                                const std::string& symbol = "");
    
    void renderFeatureCorrelationMatrix(const std::vector<FlexibleStockData>& data,
                                      const std::vector<std::string>& features,
                                      const std::string& symbol = "");
    
    // Configuration and layout
    void setDashboardLayout(const DashboardLayout& layout);
    void setChartFilter(const ChartFilter& filter);
    
    DashboardLayout& getDashboardLayout() { return layout_; }
    ChartFilter& getChartFilter() { return filter_; }
    
    // Feature management
    std::vector<std::string> getAvailableFeatures(const std::vector<FlexibleStockData>& data) const;
    std::vector<FeatureCategory> getAvailableCategories(const std::vector<FlexibleStockData>& data) const;
    
    // UI Controls
    void renderFeatureSelector(const std::vector<FlexibleStockData>& data);
    void renderCategorySelector();
    void renderLayoutControls();
    void renderChartControls();
    
    // Statistics and info
    void renderDataStatistics(const std::vector<FlexibleStockData>& data);
    void renderFeatureStatistics(const std::string& feature_name,
                                const std::vector<FlexibleStockData>& data);

private:
    DashboardLayout layout_;
    ChartFilter filter_;
    
    // UI state
    std::vector<std::string> selected_features_;
    std::vector<FeatureCategory> selected_categories_;
    bool show_feature_selector_;
    bool show_category_selector_;
    bool show_layout_controls_;
    
    // Chart caching for performance
    std::unordered_map<std::string, MultiSeriesChart> chart_cache_;
    bool use_chart_cache_;
    
    // Helper methods for rendering
    void renderGridLayout(const std::vector<MultiSeriesChart>& charts);
    void renderTabLayout(const std::vector<MultiSeriesChart>& charts,
                        const std::vector<std::string>& tab_names);
    void renderAccordionLayout(const std::vector<MultiSeriesChart>& charts,
                             const std::vector<std::string>& section_names);
    
    // Chart generation helpers
    std::vector<MultiSeriesChart> generateCategoryCharts(
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol);
    
    std::vector<MultiSeriesChart> generateFeatureCharts(
        const std::vector<FlexibleStockData>& data,
        const std::vector<std::string>& features,
        const std::string& symbol);
    
    // UI helper methods
    void renderImGuiFeatureCheckbox(const std::string& feature_name, bool& selected);
    void renderImGuiCategoryCheckbox(FeatureCategory category, bool& selected);
    
    // Statistics calculations
    struct FeatureStats {
        double mean;
        double std_dev;
        double min_val;
        double max_val;
        double median;
        size_t count;
        size_t valid_count;
    };
    
    FeatureStats calculateFeatureStatistics(const std::string& feature_name,
                                          const std::vector<FlexibleStockData>& data) const;
    
    // Correlation calculations
    double calculateCorrelation(const std::string& feature1,
                              const std::string& feature2,
                              const std::vector<FlexibleStockData>& data) const;
    
    std::vector<std::vector<double>> calculateCorrelationMatrix(
        const std::vector<std::string>& features,
        const std::vector<FlexibleStockData>& data) const;
    
    // Cache management
    std::string generateCacheKey(const std::string& chart_type,
                               const std::string& identifier,
                               const std::string& symbol) const;
    
    void clearChartCache();
    bool getCachedChart(const std::string& key, MultiSeriesChart& chart) const;
    void setCachedChart(const std::string& key, const MultiSeriesChart& chart);
};

} // namespace Visualization
