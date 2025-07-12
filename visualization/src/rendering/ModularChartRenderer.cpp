#include "rendering/ModularChartRenderer.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace Visualization {

// ChartFilter implementation
bool ChartFilter::isFeatureEnabled(const std::string& feature_name) const {
    // Check if explicitly disabled
    if (std::find(disabled_features.begin(), disabled_features.end(), feature_name) != disabled_features.end()) {
        return false;
    }
    
    // If enabled_features is not empty, only show explicitly enabled features
    if (!enabled_features.empty()) {
        return std::find(enabled_features.begin(), enabled_features.end(), feature_name) != enabled_features.end();
    }
    
    // Check category-based filtering
    auto& registry = FeatureRegistry::getInstance();
    auto feature_meta = registry.getFeature(feature_name);
    if (feature_meta) {
        return isCategoryEnabled(feature_meta->category);
    }
    
    return true; // Default to enabled
}

bool ChartFilter::isCategoryEnabled(FeatureCategory category) const {
    // If enabled_categories is not empty, only show explicitly enabled categories
    if (!enabled_categories.empty()) {
        return std::find(enabled_categories.begin(), enabled_categories.end(), category) != enabled_categories.end();
    }
    
    // Check specific category flags
    switch (category) {
        case FeatureCategory::PRICE:
            return show_price_charts;
        case FeatureCategory::VOLUME:
            return show_volume_charts;
        case FeatureCategory::TECHNICAL:
        case FeatureCategory::MOMENTUM:
        case FeatureCategory::VOLATILITY:
            return show_technical_indicators;
        case FeatureCategory::STATISTICAL:
        case FeatureCategory::RISK:
        case FeatureCategory::REGIME:
        case FeatureCategory::INTERACTION:
            return show_statistical_features;
        default:
            return true;
    }
}

// ModularChartRenderer implementation
ModularChartRenderer::ModularChartRenderer()
    : layout_(DashboardLayout::LayoutType::GRID, 2, 3),
      filter_(),
      show_feature_selector_(false),
      show_category_selector_(false),
      show_layout_controls_(false),
      use_chart_cache_(true) {
    
    // Initialize with all categories enabled
    auto& registry = FeatureRegistry::getInstance();
    selected_categories_ = registry.getAllCategories();
}

void ModularChartRenderer::renderDashboard(const std::vector<FlexibleStockData>& data, 
                                         const std::string& symbol) {
    if (data.empty()) {
        ImGui::Text("No data available for visualization");
        return;
    }
    
    // Render control panels
    if (ImGui::CollapsingHeader("Dashboard Controls")) {
        if (ImGui::Button("Feature Selector")) {
            show_feature_selector_ = !show_feature_selector_;
        }
        ImGui::SameLine();
        if (ImGui::Button("Category Selector")) {
            show_category_selector_ = !show_category_selector_;
        }
        ImGui::SameLine();
        if (ImGui::Button("Layout Controls")) {
            show_layout_controls_ = !show_layout_controls_;
        }
        
        if (show_feature_selector_) {
            renderFeatureSelector(data);
        }
        if (show_category_selector_) {
            renderCategorySelector();
        }
        if (show_layout_controls_) {
            renderLayoutControls();
        }
    }
    
    // Generate charts based on current selection
    std::vector<MultiSeriesChart> charts;
    std::vector<std::string> chart_names;
    
    // Always include price and volume overview
    if (filter_.show_price_charts) {
        auto price_chart = ChartFactory::createPriceChart(data, symbol);
        charts.push_back(std::move(price_chart));
        chart_names.push_back("Price Overview");
    }
    
    if (filter_.show_volume_charts) {
        auto volume_chart = ChartFactory::createVolumeChart(data, symbol);
        charts.push_back(std::move(volume_chart));
        chart_names.push_back("Volume Analysis");
    }
    
    // Add category charts
    auto category_charts = generateCategoryCharts(data, symbol);
    charts.insert(charts.end(), category_charts.begin(), category_charts.end());
    
    for (const auto& category : selected_categories_) {
        if (filter_.isCategoryEnabled(category)) {
            auto& registry = FeatureRegistry::getInstance();
            chart_names.push_back(registry.getCategoryName(category));
        }
    }
    
    // Render based on layout type
    switch (layout_.type) {
        case DashboardLayout::LayoutType::GRID:
            renderGridLayout(charts);
            break;
        case DashboardLayout::LayoutType::TABS:
            renderTabLayout(charts, chart_names);
            break;
        case DashboardLayout::LayoutType::ACCORDION:
            renderAccordionLayout(charts, chart_names);
            break;
        default:
            renderGridLayout(charts);
            break;
    }
}

void ModularChartRenderer::renderCategoryDashboard(const std::vector<FlexibleStockData>& data,
                                                 const std::string& symbol) {
    auto charts = generateCategoryCharts(data, symbol);
    std::vector<std::string> chart_names;
    
    auto& registry = FeatureRegistry::getInstance();
    for (const auto& category : selected_categories_) {
        if (filter_.isCategoryEnabled(category)) {
            chart_names.push_back(registry.getCategoryName(category));
        }
    }
    
    renderTabLayout(charts, chart_names);
}

void ModularChartRenderer::renderCustomDashboard(const std::vector<FlexibleStockData>& data,
                                                const std::vector<std::string>& selected_features,
                                                const std::string& symbol) {
    auto charts = generateFeatureCharts(data, selected_features, symbol);
    renderGridLayout(charts);
}

void ModularChartRenderer::renderFeatureChart(const std::string& feature_name,
                                            const std::vector<FlexibleStockData>& data,
                                            const std::string& symbol) {
    auto chart = ChartFactory::createFeatureChart(feature_name, data, symbol);
    ChartFactory::renderChart(chart);
}

void ModularChartRenderer::renderCategoryChart(FeatureCategory category,
                                             const std::vector<FlexibleStockData>& data,
                                             const std::string& symbol) {
    auto chart = ChartFactory::createCategoryChart(category, data, symbol);
    ChartFactory::renderChart(chart);
}

void ModularChartRenderer::renderComparisonChart(const std::vector<std::string>& feature_names,
                                               const std::vector<FlexibleStockData>& data,
                                               const std::string& title,
                                               const std::string& symbol) {
    auto chart = ChartFactory::createComparisonChart(feature_names, data, title, symbol);
    ChartFactory::renderChart(chart);
}

void ModularChartRenderer::renderPriceVolumeOverview(const std::vector<FlexibleStockData>& data,
                                                   const std::string& symbol) {
    // Price chart
    auto price_chart = ChartFactory::createPriceChart(data, symbol);
    ChartFactory::renderChart(price_chart);
    
    // Volume chart
    auto volume_chart = ChartFactory::createVolumeChart(data, symbol);
    ChartFactory::renderChart(volume_chart);
}

void ModularChartRenderer::renderStatisticalSummary(const std::vector<FlexibleStockData>& data,
                                                  const std::string& symbol) {
    if (data.empty()) return;
    
    ImGui::Text("Statistical Summary - %s", symbol.c_str());
    ImGui::Separator();
    
    // Get available features
    auto available_features = getAvailableFeatures(data);
    
    // Display statistics in columns
    ImGui::Columns(4, "StatsColumns");
    ImGui::Text("Feature");
    ImGui::NextColumn();
    ImGui::Text("Mean");
    ImGui::NextColumn();
    ImGui::Text("Std Dev");
    ImGui::NextColumn();
    ImGui::Text("Range");
    ImGui::NextColumn();
    ImGui::Separator();
    
    for (const auto& feature : available_features) {
        if (!filter_.isFeatureEnabled(feature)) continue;
        
        auto stats = calculateFeatureStatistics(feature, data);
        if (stats.valid_count == 0) continue;
        
        ImGui::Text("%s", feature.c_str());
        ImGui::NextColumn();
        ImGui::Text("%.4f", stats.mean);
        ImGui::NextColumn();
        ImGui::Text("%.4f", stats.std_dev);
        ImGui::NextColumn();
        ImGui::Text("%.4f - %.4f", stats.min_val, stats.max_val);
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
}

void ModularChartRenderer::renderFeatureCorrelationMatrix(const std::vector<FlexibleStockData>& data,
                                                        const std::vector<std::string>& features,
                                                        const std::string& symbol) {
    if (features.size() < 2) {
        ImGui::Text("Need at least 2 features for correlation matrix");
        return;
    }
    
    auto correlation_matrix = calculateCorrelationMatrix(features, data);
    
    std::string title = "Feature Correlation Matrix";
    if (!symbol.empty()) {
        title += " - " + symbol;
    }
    
    if (ImPlot::BeginPlot(title.c_str(), ImVec2(-1, 400))) {
        ImPlot::SetupAxes("Features", "Features");
        
        // Create heatmap data
        std::vector<float> heatmap_data;
        for (const auto& row : correlation_matrix) {
            for (double val : row) {
                heatmap_data.push_back(static_cast<float>(val));
            }
        }
        
        ImPlot::PlotHeatmap("Correlation", heatmap_data.data(), 
                           features.size(), features.size(),
                           -1.0, 1.0, nullptr);
        
        ImPlot::EndPlot();
    }
}

void ModularChartRenderer::setDashboardLayout(const DashboardLayout& layout) {
    layout_ = layout;
}

void ModularChartRenderer::setChartFilter(const ChartFilter& filter) {
    filter_ = filter;
    clearChartCache(); // Clear cache when filter changes
}

std::vector<std::string> ModularChartRenderer::getAvailableFeatures(const std::vector<FlexibleStockData>& data) const {
    if (data.empty()) return {};
    
    // Get features from the first data point
    auto feature_names = data[0].getFeatureNames();
    
    // Filter based on current filter settings
    std::vector<std::string> filtered_features;
    for (const auto& feature : feature_names) {
        if (filter_.isFeatureEnabled(feature)) {
            filtered_features.push_back(feature);
        }
    }
    
    return filtered_features;
}

std::vector<FeatureCategory> ModularChartRenderer::getAvailableCategories(const std::vector<FlexibleStockData>& data) const {
    auto& registry = FeatureRegistry::getInstance();
    auto all_categories = registry.getAllCategories();
    
    std::vector<FeatureCategory> available_categories;
    for (const auto& category : all_categories) {
        if (filter_.isCategoryEnabled(category)) {
            available_categories.push_back(category);
        }
    }
    
    return available_categories;
}

void ModularChartRenderer::renderFeatureSelector(const std::vector<FlexibleStockData>& data) {
    ImGui::Begin("Feature Selector", &show_feature_selector_);
    
    auto available_features = getAvailableFeatures(data);
    
    ImGui::Text("Select features to display:");
    ImGui::Separator();
    
    for (const auto& feature : available_features) {
        bool selected = std::find(selected_features_.begin(), selected_features_.end(), feature) != selected_features_.end();
        if (ImGui::Checkbox(feature.c_str(), &selected)) {
            if (selected) {
                selected_features_.push_back(feature);
            } else {
                selected_features_.erase(
                    std::remove(selected_features_.begin(), selected_features_.end(), feature),
                    selected_features_.end());
            }
        }
    }
    
    ImGui::End();
}

void ModularChartRenderer::renderCategorySelector() {
    ImGui::Begin("Category Selector", &show_category_selector_);
    
    auto& registry = FeatureRegistry::getInstance();
    auto all_categories = registry.getAllCategories();
    
    ImGui::Text("Select categories to display:");
    ImGui::Separator();
    
    for (const auto& category : all_categories) {
        bool selected = std::find(selected_categories_.begin(), selected_categories_.end(), category) != selected_categories_.end();
        std::string category_name = registry.getCategoryName(category);
        
        if (ImGui::Checkbox(category_name.c_str(), &selected)) {
            if (selected) {
                selected_categories_.push_back(category);
            } else {
                selected_categories_.erase(
                    std::remove(selected_categories_.begin(), selected_categories_.end(), category),
                    selected_categories_.end());
            }
        }
    }
    
    ImGui::End();
}

void ModularChartRenderer::renderLayoutControls() {
    ImGui::Begin("Layout Controls", &show_layout_controls_);
    
    // Layout type selection
    const char* layout_types[] = {"Grid", "Tabs", "Accordion", "Custom"};
    int current_layout = static_cast<int>(layout_.type);
    if (ImGui::Combo("Layout Type", &current_layout, layout_types, 4)) {
        layout_.type = static_cast<DashboardLayout::LayoutType>(current_layout);
    }
    
    // Grid layout settings
    if (layout_.type == DashboardLayout::LayoutType::GRID) {
        ImGui::SliderInt("Columns", &layout_.columns, 1, 4);
        ImGui::SliderInt("Rows", &layout_.rows, 1, 6);
    }
    
    ImGui::Checkbox("Auto Resize", &layout_.auto_resize);
    ImGui::SliderFloat("Chart Spacing", &layout_.chart_spacing, 5.0f, 50.0f);
    
    // Filter controls
    ImGui::Separator();
    ImGui::Text("Chart Filters:");
    ImGui::Checkbox("Show Price Charts", &filter_.show_price_charts);
    ImGui::Checkbox("Show Volume Charts", &filter_.show_volume_charts);
    ImGui::Checkbox("Show Technical Indicators", &filter_.show_technical_indicators);
    ImGui::Checkbox("Show Statistical Features", &filter_.show_statistical_features);
    
    ImGui::End();
}

void ModularChartRenderer::renderChartControls() {
    // Additional chart-specific controls can be added here
    ImGui::Text("Chart Controls - Coming Soon");
}

void ModularChartRenderer::renderDataStatistics(const std::vector<FlexibleStockData>& data) {
    if (data.empty()) return;
    
    ImGui::Text("Data Statistics:");
    ImGui::Text("Total Data Points: %zu", data.size());
    ImGui::Text("Available Features: %zu", data[0].getFeatureNames().size());
    
    if (!data.empty()) {
        ImGui::Text("Date Range: %s to %s", 
                   data.front().date_string.c_str(),
                   data.back().date_string.c_str());
    }
}

void ModularChartRenderer::renderFeatureStatistics(const std::string& feature_name,
                                                  const std::vector<FlexibleStockData>& data) {
    auto stats = calculateFeatureStatistics(feature_name, data);
    
    ImGui::Text("Feature: %s", feature_name.c_str());
    ImGui::Text("Valid Points: %zu / %zu", stats.valid_count, stats.count);
    ImGui::Text("Mean: %.6f", stats.mean);
    ImGui::Text("Std Dev: %.6f", stats.std_dev);
    ImGui::Text("Min: %.6f", stats.min_val);
    ImGui::Text("Max: %.6f", stats.max_val);
    ImGui::Text("Median: %.6f", stats.median);
}

// Private helper methods implementation

void ModularChartRenderer::renderGridLayout(const std::vector<MultiSeriesChart>& charts) {
    if (charts.empty()) return;
    
    int cols = layout_.columns;
    int rows = layout_.rows;
    
    for (int row = 0; row < rows && row * cols < static_cast<int>(charts.size()); ++row) {
        for (int col = 0; col < cols && row * cols + col < static_cast<int>(charts.size()); ++col) {
            if (col > 0) ImGui::SameLine();
            
            int chart_index = row * cols + col;
            ChartFactory::renderChart(charts[chart_index]);
        }
    }
}

void ModularChartRenderer::renderTabLayout(const std::vector<MultiSeriesChart>& charts,
                                         const std::vector<std::string>& tab_names) {
    if (charts.empty()) return;
    
    if (ImGui::BeginTabBar("ChartTabs")) {
        for (size_t i = 0; i < charts.size() && i < tab_names.size(); ++i) {
            if (ImGui::BeginTabItem(tab_names[i].c_str())) {
                ChartFactory::renderChart(charts[i]);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void ModularChartRenderer::renderAccordionLayout(const std::vector<MultiSeriesChart>& charts,
                                                const std::vector<std::string>& section_names) {
    for (size_t i = 0; i < charts.size() && i < section_names.size(); ++i) {
        if (ImGui::CollapsingHeader(section_names[i].c_str())) {
            ChartFactory::renderChart(charts[i]);
        }
    }
}

std::vector<MultiSeriesChart> ModularChartRenderer::generateCategoryCharts(
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    std::vector<MultiSeriesChart> charts;
    
    for (const auto& category : selected_categories_) {
        if (filter_.isCategoryEnabled(category)) {
            auto chart = ChartFactory::createCategoryChart(category, data, symbol);
            if (!chart.series.empty()) {
                charts.push_back(std::move(chart));
            }
        }
    }
    
    return charts;
}

std::vector<MultiSeriesChart> ModularChartRenderer::generateFeatureCharts(
    const std::vector<FlexibleStockData>& data,
    const std::vector<std::string>& features,
    const std::string& symbol) {
    
    std::vector<MultiSeriesChart> charts;
    
    for (const auto& feature : features) {
        if (filter_.isFeatureEnabled(feature)) {
            auto chart = ChartFactory::createFeatureChart(feature, data, symbol);
            if (!chart.series.empty()) {
                charts.push_back(std::move(chart));
            }
        }
    }
    
    return charts;
}

ModularChartRenderer::FeatureStats ModularChartRenderer::calculateFeatureStatistics(
    const std::string& feature_name,
    const std::vector<FlexibleStockData>& data) const {
    
    FeatureStats stats = {};
    stats.count = data.size();
    
    std::vector<double> valid_values;
    for (const auto& data_point : data) {
        if (data_point.hasFeature(feature_name)) {
            double value = data_point.getFeature(feature_name);
            if (!std::isnan(value) && !std::isinf(value)) {
                valid_values.push_back(value);
            }
        }
    }
    
    stats.valid_count = valid_values.size();
    if (valid_values.empty()) return stats;
    
    // Calculate mean
    stats.mean = std::accumulate(valid_values.begin(), valid_values.end(), 0.0) / valid_values.size();
    
    // Calculate standard deviation
    double variance = 0.0;
    for (double value : valid_values) {
        variance += (value - stats.mean) * (value - stats.mean);
    }
    stats.std_dev = std::sqrt(variance / valid_values.size());
    
    // Calculate min/max
    auto minmax = std::minmax_element(valid_values.begin(), valid_values.end());
    stats.min_val = *minmax.first;
    stats.max_val = *minmax.second;
    
    // Calculate median
    std::sort(valid_values.begin(), valid_values.end());
    size_t n = valid_values.size();
    if (n % 2 == 0) {
        stats.median = (valid_values[n/2 - 1] + valid_values[n/2]) / 2.0;
    } else {
        stats.median = valid_values[n/2];
    }
    
    return stats;
}

double ModularChartRenderer::calculateCorrelation(const std::string& feature1,
                                                const std::string& feature2,
                                                const std::vector<FlexibleStockData>& data) const {
    std::vector<double> values1, values2;
    
    for (const auto& data_point : data) {
        if (data_point.hasFeature(feature1) && data_point.hasFeature(feature2)) {
            double val1 = data_point.getFeature(feature1);
            double val2 = data_point.getFeature(feature2);
            
            if (!std::isnan(val1) && !std::isinf(val1) && !std::isnan(val2) && !std::isinf(val2)) {
                values1.push_back(val1);
                values2.push_back(val2);
            }
        }
    }
    
    if (values1.size() < 2) return 0.0;
    
    // Calculate Pearson correlation coefficient
    double mean1 = std::accumulate(values1.begin(), values1.end(), 0.0) / values1.size();
    double mean2 = std::accumulate(values2.begin(), values2.end(), 0.0) / values2.size();
    
    double numerator = 0.0, sum_sq1 = 0.0, sum_sq2 = 0.0;
    
    for (size_t i = 0; i < values1.size(); ++i) {
        double diff1 = values1[i] - mean1;
        double diff2 = values2[i] - mean2;
        numerator += diff1 * diff2;
        sum_sq1 += diff1 * diff1;
        sum_sq2 += diff2 * diff2;
    }
    
    double denominator = std::sqrt(sum_sq1 * sum_sq2);
    return (denominator > 0.0) ? numerator / denominator : 0.0;
}

std::vector<std::vector<double>> ModularChartRenderer::calculateCorrelationMatrix(
    const std::vector<std::string>& features,
    const std::vector<FlexibleStockData>& data) const {
    
    size_t n = features.size();
    std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0.0));
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i == j) {
                matrix[i][j] = 1.0; // Perfect correlation with self
            } else {
                matrix[i][j] = calculateCorrelation(features[i], features[j], data);
            }
        }
    }
    
    return matrix;
}

std::string ModularChartRenderer::generateCacheKey(const std::string& chart_type,
                                                 const std::string& identifier,
                                                 const std::string& symbol) const {
    return chart_type + "_" + identifier + "_" + symbol;
}

void ModularChartRenderer::clearChartCache() {
    chart_cache_.clear();
}

bool ModularChartRenderer::getCachedChart(const std::string& key, MultiSeriesChart& chart) const {
    if (!use_chart_cache_) return false;
    
    auto it = chart_cache_.find(key);
    if (it != chart_cache_.end()) {
        chart = it->second;
        return true;
    }
    return false;
}

void ModularChartRenderer::setCachedChart(const std::string& key, const MultiSeriesChart& chart) {
    if (use_chart_cache_) {
        chart_cache_[key] = chart;
    }
}

} // namespace Visualization
