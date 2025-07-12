#include "core/ChartFactory.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <cmath>

namespace Visualization {

MultiSeriesChart ChartFactory::createFeatureChart(
    const std::string& feature_name,
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    auto config = createConfigForFeature(feature_name, symbol);
    MultiSeriesChart chart(config);
    
    auto feature_values = extractFeatureValues(data, feature_name);
    auto time_indices = extractTimeIndices(data);
    
    if (!feature_values.empty() && feature_values.size() == time_indices.size()) {
        ChartData chart_data(feature_name);
        chart_data.x_values = time_indices;
        chart_data.y_values = feature_values;
        
        // Get chart type from registry
        auto& registry = FeatureRegistry::getInstance();
        auto feature_meta = registry.getFeature(feature_name);
        if (feature_meta) {
            chart_data.chart_type = feature_meta->preferred_chart_type;
        }
        
        applyFeatureColor(chart_data, feature_name);
        chart.addSeries(std::move(chart_data));
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createCategoryChart(
    FeatureCategory category,
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    auto config = createConfigForCategory(category, symbol);
    MultiSeriesChart chart(config);
    
    auto& registry = FeatureRegistry::getInstance();
    auto feature_names = registry.getFeaturesByCategory(category);
    auto time_indices = extractTimeIndices(data);
    
    std::vector<ChartData> chart_data_list;
    
    for (const auto& feature_name : feature_names) {
        auto feature_values = extractFeatureValues(data, feature_name);
        
        if (!feature_values.empty() && feature_values.size() == time_indices.size()) {
            ChartData chart_data(feature_name);
            chart_data.x_values = time_indices;
            chart_data.y_values = feature_values;
            
            auto feature_meta = registry.getFeature(feature_name);
            if (feature_meta) {
                chart_data.chart_type = feature_meta->preferred_chart_type;
            }
            
            chart_data_list.push_back(std::move(chart_data));
        }
    }
    
    // Apply category-based colors
    applyCategoryColors(chart_data_list, category);
    
    for (auto& data : chart_data_list) {
        chart.addSeries(std::move(data));
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createComparisonChart(
    const std::vector<std::string>& feature_names,
    const std::vector<FlexibleStockData>& data,
    const std::string& title,
    const std::string& symbol) {
    
    ChartConfig config(title.empty() ? "Feature Comparison" : title);
    if (!symbol.empty()) {
        config.title += " - " + symbol;
    }
    
    MultiSeriesChart chart(config);
    auto time_indices = extractTimeIndices(data);
    
    for (size_t i = 0; i < feature_names.size(); ++i) {
        const auto& feature_name = feature_names[i];
        auto feature_values = extractFeatureValues(data, feature_name);
        
        if (!feature_values.empty() && feature_values.size() == time_indices.size()) {
            ChartData chart_data(feature_name);
            chart_data.x_values = time_indices;
            chart_data.y_values = feature_values;
            
            applyFeatureColor(chart_data, feature_name);
            chart.addSeries(std::move(chart_data));
        }
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createOHLCChart(
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    ChartConfig config("OHLC Price Chart");
    if (!symbol.empty()) {
        config.title += " - " + symbol;
    }
    config.y_axis_label = "Price ($)";
    config.height = 300.0f;
    
    MultiSeriesChart chart(config);
    auto time_indices = extractTimeIndices(data);
    
    // Add OHLC series
    std::vector<std::string> price_features = {"open", "high", "low", "close"};
    std::vector<std::string> display_names = {"Open", "High", "Low", "Close"};
    float colors[4][3] = {
        {0.2f, 0.6f, 1.0f},  // Blue for Open
        {0.0f, 0.8f, 0.0f},  // Green for High
        {1.0f, 0.0f, 0.0f},  // Red for Low
        {0.8f, 0.4f, 0.0f}   // Orange for Close
    };
    
    for (size_t i = 0; i < price_features.size(); ++i) {
        auto values = extractFeatureValues(data, price_features[i]);
        if (!values.empty() && values.size() == time_indices.size()) {
            ChartData chart_data(display_names[i]);
            chart_data.x_values = time_indices;
            chart_data.y_values = values;
            chart_data.setColor(colors[i][0], colors[i][1], colors[i][2]);
            chart.addSeries(std::move(chart_data));
        }
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createVolumeChart(
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    ChartConfig config("Volume Analysis");
    if (!symbol.empty()) {
        config.title += " - " + symbol;
    }
    config.y_axis_label = "Volume";
    config.height = 200.0f;
    
    MultiSeriesChart chart(config);
    auto time_indices = extractTimeIndices(data);
    
    // Volume bars
    auto volume_values = extractFeatureValues(data, "volume");
    if (!volume_values.empty() && volume_values.size() == time_indices.size()) {
        ChartData volume_data("Volume", ChartType::BAR);
        volume_data.x_values = time_indices;
        volume_data.y_values = volume_values;
        volume_data.setColor(0.0f, 0.8f, 0.4f); // Green
        chart.addSeries(std::move(volume_data));
    }
    
    // Volume SMA line
    auto volume_sma_values = extractFeatureValues(data, "volume_sma_20");
    if (!volume_sma_values.empty() && volume_sma_values.size() == time_indices.size()) {
        ChartData sma_data("Volume SMA 20");
        sma_data.x_values = time_indices;
        sma_data.y_values = volume_sma_values;
        sma_data.setColor(1.0f, 0.5f, 0.0f); // Orange
        chart.addSeries(std::move(sma_data));
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createPriceChart(
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    ChartConfig config("Price Chart");
    if (!symbol.empty()) {
        config.title += " - " + symbol;
    }
    config.y_axis_label = "Price ($)";
    config.height = 300.0f;
    
    MultiSeriesChart chart(config);
    auto time_indices = extractTimeIndices(data);
    
    // Close price
    auto close_values = extractFeatureValues(data, "close");
    if (!close_values.empty() && close_values.size() == time_indices.size()) {
        ChartData close_data("Close Price");
        close_data.x_values = time_indices;
        close_data.y_values = close_values;
        close_data.setColor(0.2f, 0.6f, 1.0f); // Blue
        chart.addSeries(std::move(close_data));
    }
    
    // SMA
    auto sma_values = extractFeatureValues(data, "sma");
    if (!sma_values.empty() && sma_values.size() == time_indices.size()) {
        ChartData sma_data("SMA");
        sma_data.x_values = time_indices;
        sma_data.y_values = sma_values;
        sma_data.setColor(1.0f, 0.5f, 0.0f); // Orange
        chart.addSeries(std::move(sma_data));
    }
    
    return chart;
}

MultiSeriesChart ChartFactory::createDistributionChart(
    const std::string& feature_name,
    const std::vector<FlexibleStockData>& data,
    const std::string& symbol) {
    
    ChartConfig config("Distribution: " + feature_name);
    if (!symbol.empty()) {
        config.title += " - " + symbol;
    }
    config.x_axis_label = "Value";
    config.y_axis_label = "Frequency";
    config.height = 250.0f;
    
    MultiSeriesChart chart(config);
    
    auto feature_values = extractFeatureValues(data, feature_name);
    if (feature_values.empty()) {
        return chart;
    }
    
    // Create histogram
    const int num_bins = 50;
    auto min_val = *std::min_element(feature_values.begin(), feature_values.end());
    auto max_val = *std::max_element(feature_values.begin(), feature_values.end());
    
    if (min_val == max_val) {
        return chart; // No variation in data
    }
    
    float bin_width = (max_val - min_val) / num_bins;
    std::vector<float> bin_centers(num_bins);
    std::vector<float> bin_counts(num_bins, 0.0f);
    
    // Calculate bin centers
    for (int i = 0; i < num_bins; ++i) {
        bin_centers[i] = min_val + (i + 0.5f) * bin_width;
    }
    
    // Count values in each bin
    for (float value : feature_values) {
        int bin_index = static_cast<int>((value - min_val) / bin_width);
        bin_index = std::max(0, std::min(num_bins - 1, bin_index));
        bin_counts[bin_index] += 1.0f;
    }
    
    ChartData hist_data(feature_name + " Distribution", ChartType::BAR);
    hist_data.x_values = bin_centers;
    hist_data.y_values = bin_counts;
    applyFeatureColor(hist_data, feature_name);
    
    chart.addSeries(std::move(hist_data));
    return chart;
}

void ChartFactory::renderChart(const MultiSeriesChart& chart) {
    if (chart.series.empty()) {
        ImGui::Text("No data available for chart: %s", chart.config.title.c_str());
        return;
    }
    
    ImVec2 plot_size(chart.config.width, chart.config.height);
    
    if (ImPlot::BeginPlot(chart.config.title.c_str(), plot_size)) {
        setupChartAxes(chart.config);
        setupChartLimits(chart.config);
        
        // Use the first series' x_values as the common x-axis
        const auto& x_values = chart.series[0].x_values;
        
        for (const auto& series : chart.series) {
            if (series.y_values.size() != x_values.size()) {
                continue; // Skip mismatched series
            }
            
            // Set series color
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(series.color[0], series.color[1], series.color[2], 1.0f));
            ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(series.color[0], series.color[1], series.color[2], 0.3f));
            
            switch (series.chart_type) {
                case ChartType::LINE:
                    renderLineChart(series, x_values);
                    break;
                case ChartType::BAR:
                    renderBarChart(series, x_values);
                    break;
                case ChartType::AREA:
                    renderAreaChart(series, x_values);
                    break;
                case ChartType::SCATTER:
                    renderScatterChart(series, x_values);
                    break;
                default:
                    renderLineChart(series, x_values);
                    break;
            }
            
            ImPlot::PopStyleColor(2);
        }
        
        ImPlot::EndPlot();
    }
}

std::vector<float> ChartFactory::extractFeatureValues(
    const std::vector<FlexibleStockData>& data,
    const std::string& feature_name) {
    
    std::vector<float> values;
    values.reserve(data.size());
    
    for (const auto& data_point : data) {
        if (data_point.hasFeature(feature_name)) {
            double value = data_point.getFeature(feature_name);
            if (!std::isnan(value) && !std::isinf(value)) {
                values.push_back(static_cast<float>(value));
            } else {
                values.push_back(0.0f); // Replace invalid values with 0
            }
        } else {
            values.push_back(0.0f); // Missing feature
        }
    }
    
    return values;
}

std::vector<float> ChartFactory::extractTimeIndices(
    const std::vector<FlexibleStockData>& data) {
    
    std::vector<float> indices;
    indices.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        indices.push_back(static_cast<float>(i));
    }
    
    return indices;
}

ChartConfig ChartFactory::createConfigForFeature(
    const std::string& feature_name,
    const std::string& symbol) {
    
    auto& registry = FeatureRegistry::getInstance();
    auto feature_meta = registry.getFeature(feature_name);
    
    std::string title = feature_name;
    std::string y_label = "Value";
    
    if (feature_meta) {
        title = feature_meta->display_name;
        if (!feature_meta->units.empty()) {
            y_label = feature_meta->units;
        }
        if (feature_meta->is_percentage) {
            y_label += " (%)";
        }
    }
    
    if (!symbol.empty()) {
        title += " - " + symbol;
    }
    
    ChartConfig config(title);
    config.y_axis_label = y_label;
    
    if (feature_meta && feature_meta->has_bounds) {
        config.setYBounds(feature_meta->min_bound, feature_meta->max_bound);
    }
    
    return config;
}

ChartConfig ChartFactory::createConfigForCategory(
    FeatureCategory category,
    const std::string& symbol) {
    
    auto& registry = FeatureRegistry::getInstance();
    std::string title = registry.getCategoryName(category);
    
    if (!symbol.empty()) {
        title += " - " + symbol;
    }
    
    ChartConfig config(title);
    config.height = 250.0f; // Slightly larger for multiple series
    
    return config;
}

void ChartFactory::renderLineChart(const ChartData& data, const std::vector<float>& x_values) {
    ImPlot::PlotLine(data.series_name.c_str(), x_values.data(), data.y_values.data(), x_values.size());
}

void ChartFactory::renderBarChart(const ChartData& data, const std::vector<float>& x_values) {
    ImPlot::PlotBars(data.series_name.c_str(), x_values.data(), data.y_values.data(), x_values.size(), 0.8);
}

void ChartFactory::renderAreaChart(const ChartData& data, const std::vector<float>& x_values) {
    ImPlot::PlotShaded(data.series_name.c_str(), x_values.data(), data.y_values.data(), x_values.size());
}

void ChartFactory::renderScatterChart(const ChartData& data, const std::vector<float>& x_values) {
    ImPlot::PlotScatter(data.series_name.c_str(), x_values.data(), data.y_values.data(), x_values.size());
}

void ChartFactory::applyFeatureColor(ChartData& chart_data, const std::string& feature_name) {
    auto& registry = FeatureRegistry::getInstance();
    auto feature_meta = registry.getFeature(feature_name);
    
    if (feature_meta) {
        chart_data.setColor(feature_meta->color[0], feature_meta->color[1], feature_meta->color[2]);
    }
}

void ChartFactory::applyCategoryColors(std::vector<ChartData>& chart_data_list, FeatureCategory category) {
    // Generate colors based on category base color with variations
    auto& registry = FeatureRegistry::getInstance();
    
    for (size_t i = 0; i < chart_data_list.size(); ++i) {
        auto& chart_data = chart_data_list[i];
        auto feature_meta = registry.getFeature(chart_data.series_name);
        
        if (feature_meta) {
            // Use the feature's registered color
            chart_data.setColor(feature_meta->color[0], feature_meta->color[1], feature_meta->color[2]);
        } else {
            // Generate a color variation based on index
            float hue_offset = (static_cast<float>(i) / chart_data_list.size()) * 0.3f; // 30% hue range
            float base_hue = 0.0f;
            
            switch (category) {
                case FeatureCategory::TECHNICAL: base_hue = 0.08f; break; // Orange base
                case FeatureCategory::STATISTICAL: base_hue = 0.75f; break; // Purple base
                case FeatureCategory::VOLUME: base_hue = 0.33f; break; // Green base
                default: base_hue = 0.0f; break;
            }
            
            float hue = base_hue + hue_offset;
            if (hue > 1.0f) hue -= 1.0f;
            
            // Convert HSV to RGB (simplified)
            float r, g, b;
            if (hue < 0.33f) {
                r = 1.0f - 3.0f * hue;
                g = 3.0f * hue;
                b = 0.0f;
            } else if (hue < 0.67f) {
                r = 0.0f;
                g = 2.0f - 3.0f * hue;
                b = 3.0f * hue - 1.0f;
            } else {
                r = 3.0f * hue - 2.0f;
                g = 0.0f;
                b = 3.0f - 3.0f * hue;
            }
            
            chart_data.setColor(r, g, b);
        }
    }
}

void ChartFactory::setupChartAxes(const ChartConfig& config) {
    ImPlot::SetupAxes(config.x_axis_label.c_str(), config.y_axis_label.c_str());
}

void ChartFactory::setupChartLimits(const ChartConfig& config) {
    if (config.has_y_bounds) {
        ImPlot::SetupAxisLimits(ImAxis_Y1, config.min_y, config.max_y);
    }
}

} // namespace Visualization
