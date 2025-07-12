#pragma once

#include "FeatureRegistry.h"
#include "FeatureExtractor.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace Visualization {

// Chart configuration structure
struct ChartConfig {
    std::string title;
    std::string x_axis_label;
    std::string y_axis_label;
    float width;
    float height;
    bool auto_fit;
    bool show_legend;
    float min_y;
    float max_y;
    bool has_y_bounds;
    
    ChartConfig(const std::string& title = "", 
               float width = -1.0f, 
               float height = 200.0f)
        : title(title), x_axis_label("Time Index"), y_axis_label("Value"),
          width(width), height(height), auto_fit(true), show_legend(true),
          min_y(0.0f), max_y(100.0f), has_y_bounds(false) {}
    
    void setYBounds(float min_val, float max_val) {
        min_y = min_val;
        max_y = max_val;
        has_y_bounds = true;
    }
};

// Chart data structure for rendering
struct ChartData {
    std::vector<float> x_values;
    std::vector<float> y_values;
    std::string series_name;
    float color[3];
    ChartType chart_type;
    
    ChartData(const std::string& name, ChartType type = ChartType::LINE)
        : series_name(name), chart_type(type) {
        color[0] = 0.5f; color[1] = 0.5f; color[2] = 0.5f; // Default gray
    }
    
    void setColor(float r, float g, float b) {
        color[0] = r; color[1] = g; color[2] = b;
    }
};

// Multi-series chart for combining related features
struct MultiSeriesChart {
    ChartConfig config;
    std::vector<ChartData> series;
    
    MultiSeriesChart(const ChartConfig& cfg) : config(cfg) {}
    
    void addSeries(const ChartData& data) {
        series.push_back(data);
    }
    
    void addSeries(ChartData&& data) {
        series.push_back(std::move(data));
    }
};

class ChartFactory {
public:
    // Create a single feature chart
    static MultiSeriesChart createFeatureChart(
        const std::string& feature_name,
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Create a category-based chart with multiple features
    static MultiSeriesChart createCategoryChart(
        FeatureCategory category,
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Create a comparison chart for multiple features
    static MultiSeriesChart createComparisonChart(
        const std::vector<std::string>& feature_names,
        const std::vector<FlexibleStockData>& data,
        const std::string& title = "Feature Comparison",
        const std::string& symbol = "");
    
    // Create OHLC candlestick chart
    static MultiSeriesChart createOHLCChart(
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Create volume chart with bars
    static MultiSeriesChart createVolumeChart(
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Create price chart with multiple price series
    static MultiSeriesChart createPriceChart(
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Create statistical distribution chart
    static MultiSeriesChart createDistributionChart(
        const std::string& feature_name,
        const std::vector<FlexibleStockData>& data,
        const std::string& symbol = "");
    
    // Render a multi-series chart using ImPlot
    static void renderChart(const MultiSeriesChart& chart);
    
    // Utility functions
    static std::vector<float> extractFeatureValues(
        const std::vector<FlexibleStockData>& data,
        const std::string& feature_name);
    
    static std::vector<float> extractTimeIndices(
        const std::vector<FlexibleStockData>& data);
    
    static ChartConfig createConfigForFeature(
        const std::string& feature_name,
        const std::string& symbol = "");
    
    static ChartConfig createConfigForCategory(
        FeatureCategory category,
        const std::string& symbol = "");

private:
    // Helper functions for specific chart types
    static void renderLineChart(const ChartData& data, const std::vector<float>& x_values);
    static void renderBarChart(const ChartData& data, const std::vector<float>& x_values);
    static void renderAreaChart(const ChartData& data, const std::vector<float>& x_values);
    static void renderScatterChart(const ChartData& data, const std::vector<float>& x_values);
    
    // Color management
    static void applyFeatureColor(ChartData& chart_data, const std::string& feature_name);
    static void applyCategoryColors(std::vector<ChartData>& chart_data, FeatureCategory category);
    
    // Chart styling
    static void setupChartAxes(const ChartConfig& config);
    static void setupChartLimits(const ChartConfig& config);
};

} // namespace Visualization
