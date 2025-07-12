#include "VisualizationManager.h"
#include "imgui.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace Visualization {

// DataManager implementation
std::vector<FlexibleStockData> DataManager::loadFromCSV(const std::string& csv_path) {
    std::vector<FlexibleStockData> data;
    std::ifstream file(csv_path);
    
    if (!file.is_open()) {
        return data; // Return empty vector on error
    }
    
    std::string line;
    std::vector<std::string> headers;
    
    // Read header line
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string header;
        
        while (std::getline(ss, header, ',')) {
            // Remove quotes and whitespace
            header.erase(std::remove(header.begin(), header.end(), '"'), header.end());
            header.erase(std::remove(header.begin(), header.end(), ' '), header.end());
            headers.push_back(header);
        }
    }
    
    // Extract symbol from filename
    std::string symbol = extractSymbolFromPath(csv_path);
    
    // Read data lines
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;
        
        while (std::getline(ss, cell, ',')) {
            // Remove quotes
            cell.erase(std::remove(cell.begin(), cell.end(), '"'), cell.end());
            row.push_back(cell);
        }
        
        if (row.size() != headers.size()) {
            continue; // Skip malformed rows
        }
        
        FlexibleStockData data_point;
        data_point.symbol = symbol;
        
        // Parse each column
        for (size_t i = 0; i < headers.size() && i < row.size(); ++i) {
            const std::string& header = headers[i];
            const std::string& value_str = row[i];
            
            try {
                if (header == "date" || header == "Date") {
                    data_point.date_string = value_str;
                } else if (header == "datetime_index") {
                    data_point.datetime_index = std::stof(value_str);
                } else {
                    // Try to parse as numeric feature
                    double value = std::stod(value_str);
                    data_point.setFeature(header, value);
                }
            } catch (const std::exception&) {
                // Skip invalid values
                continue;
            }
        }
        
        data.push_back(std::move(data_point));
    }
    
    return data;
}

std::vector<FlexibleStockData> DataManager::convertFromFeatureSet(
    const std::string& symbol,
    const OHLCVData& ohlcv_data,
    const FeatureSet& feature_set) {
    
    return FeatureExtractor::extractFromFeatureSet(symbol, ohlcv_data, feature_set);
}

std::unordered_map<std::string, std::vector<FlexibleStockData>> DataManager::loadMultipleSymbols(
    const std::vector<std::string>& csv_paths) {
    
    std::unordered_map<std::string, std::vector<FlexibleStockData>> result;
    
    for (const auto& path : csv_paths) {
        auto data = loadFromCSV(path);
        if (!data.empty()) {
            std::string symbol = extractSymbolFromPath(path);
            result[symbol] = std::move(data);
        }
    }
    
    return result;
}

void DataManager::validateData(std::vector<FlexibleStockData>& data) {
    // Remove data points with no features
    data.erase(
        std::remove_if(data.begin(), data.end(),
            [](const FlexibleStockData& point) {
                return point.getFeatureNames().empty();
            }),
        data.end()
    );
}

void DataManager::cleanData(std::vector<FlexibleStockData>& data, bool remove_outliers) {
    if (data.empty()) return;
    
    // Get all feature names
    auto feature_names = data[0].getFeatureNames();
    
    for (const auto& feature_name : feature_names) {
        std::vector<double> values;
        
        // Collect all valid values for this feature
        for (const auto& data_point : data) {
            if (data_point.hasFeature(feature_name)) {
                double value = data_point.getFeature(feature_name);
                if (!std::isnan(value) && !std::isinf(value)) {
                    values.push_back(value);
                }
            }
        }
        
        if (values.empty()) continue;
        
        // Calculate statistics for outlier detection
        if (remove_outliers && values.size() > 10) {
            std::sort(values.begin(), values.end());
            
            size_t q1_idx = values.size() / 4;
            size_t q3_idx = 3 * values.size() / 4;
            
            double q1 = values[q1_idx];
            double q3 = values[q3_idx];
            double iqr = q3 - q1;
            
            double lower_bound = q1 - 1.5 * iqr;
            double upper_bound = q3 + 1.5 * iqr;
            
            // Remove outliers
            for (auto& data_point : data) {
                if (data_point.hasFeature(feature_name)) {
                    double value = data_point.getFeature(feature_name);
                    if (value < lower_bound || value > upper_bound) {
                        // Set to NaN to mark as invalid
                        data_point.setFeature(feature_name, std::numeric_limits<double>::quiet_NaN());
                    }
                }
            }
        }
    }
}

std::string DataManager::extractSymbolFromPath(const std::string& csv_path) {
    std::filesystem::path path(csv_path);
    std::string filename = path.stem().string();
    
    // Remove "_features" suffix if present
    size_t pos = filename.find("_features");
    if (pos != std::string::npos) {
        filename = filename.substr(0, pos);
    }
    
    return filename;
}

// VisualizationManager implementation
VisualizationManager::VisualizationManager()
    : renderer_(),
      current_symbol_(""),
      show_symbol_selector_(false),
      show_feature_dashboard_(false),
      show_comparison_dashboard_(false),
      show_statistics_dashboard_(false),
      show_error_popup_(false),
      show_info_popup_(false) {
}

void VisualizationManager::initialize() {
    initializeFeatureRegistry();
}

void VisualizationManager::initializeFeatureRegistry() {
    // Register all features from the FeatureSet
    registerAllFeatures();
}

bool VisualizationManager::loadData(const std::string& csv_path) {
    try {
        auto data = DataManager::loadFromCSV(csv_path);
        if (data.empty()) {
            showErrorMessage("Failed to load data from: " + csv_path);
            return false;
        }
        
        DataManager::validateData(data);
        DataManager::cleanData(data, false); // Don't remove outliers by default
        
        std::string symbol = DataManager::extractSymbolFromPath(csv_path);
        symbol_data_[symbol] = std::move(data);
        
        if (current_symbol_.empty()) {
            current_symbol_ = symbol;
        }
        
        showInfoMessage("Successfully loaded " + std::to_string(symbol_data_[symbol].size()) + 
                       " data points for " + symbol);
        return true;
        
    } catch (const std::exception& e) {
        showErrorMessage("Error loading data: " + std::string(e.what()));
        return false;
    }
}

bool VisualizationManager::loadData(const std::string& symbol, 
                                   const OHLCVData& ohlcv_data, 
                                   const FeatureSet& feature_set) {
    try {
        auto data = DataManager::convertFromFeatureSet(symbol, ohlcv_data, feature_set);
        if (data.empty()) {
            showErrorMessage("Failed to convert FeatureSet data for: " + symbol);
            return false;
        }
        
        DataManager::validateData(data);
        symbol_data_[symbol] = std::move(data);
        
        if (current_symbol_.empty()) {
            current_symbol_ = symbol;
        }
        
        showInfoMessage("Successfully loaded " + std::to_string(symbol_data_[symbol].size()) + 
                       " data points for " + symbol);
        return true;
        
    } catch (const std::exception& e) {
        showErrorMessage("Error converting FeatureSet: " + std::string(e.what()));
        return false;
    }
}

bool VisualizationManager::loadMultipleDataSources(const std::vector<std::string>& csv_paths) {
    auto loaded_data = DataManager::loadMultipleSymbols(csv_paths);
    
    if (loaded_data.empty()) {
        showErrorMessage("Failed to load any data from provided paths");
        return false;
    }
    
    // Clean and validate all loaded data
    for (auto& [symbol, data] : loaded_data) {
        DataManager::validateData(data);
        DataManager::cleanData(data, false);
    }
    
    // Merge with existing data
    for (auto& [symbol, data] : loaded_data) {
        symbol_data_[symbol] = std::move(data);
    }
    
    if (current_symbol_.empty() && !symbol_data_.empty()) {
        current_symbol_ = symbol_data_.begin()->first;
    }
    
    showInfoMessage("Successfully loaded data for " + std::to_string(loaded_data.size()) + " symbols");
    return true;
}

void VisualizationManager::setCurrentSymbol(const std::string& symbol) {
    if (hasDataForSymbol(symbol)) {
        current_symbol_ = symbol;
    }
}

void VisualizationManager::renderMainDashboard() {
    renderMainMenuBar();
    
    if (symbol_data_.empty()) {
        ImGui::Begin("No Data");
        ImGui::Text("No data loaded. Please load data files first.");
        if (ImGui::Button("Load Demo Data")) {
            loadDemoData();
        }
        ImGui::End();
        return;
    }
    
    // Render symbol selector if multiple symbols available
    if (symbol_data_.size() > 1) {
        renderSymbolCombo();
    }
    
    // Render data info
    renderDataInfo();
    
    // Render main dashboard
    if (!current_symbol_.empty() && hasDataForSymbol(current_symbol_)) {
        const auto& data = getCurrentData();
        renderer_.renderDashboard(data, current_symbol_);
    }
    
    // Render additional windows
    if (show_symbol_selector_) {
        renderSymbolSelector();
    }
    if (show_feature_dashboard_) {
        renderFeatureDashboard();
    }
    if (show_comparison_dashboard_) {
        renderComparisonDashboard();
    }
    if (show_statistics_dashboard_) {
        renderStatisticsDashboard();
    }
    
    renderStatusBar();
    
    // Handle popups
    if (show_error_popup_) {
        ImGui::OpenPopup("Error");
        show_error_popup_ = false;
    }
    if (show_info_popup_) {
        ImGui::OpenPopup("Info");
        show_info_popup_ = false;
    }
    
    // Error popup
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", last_error_message_.c_str());
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Info popup
    if (ImGui::BeginPopupModal("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", last_info_message_.c_str());
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void VisualizationManager::renderSymbolSelector() {
    ImGui::Begin("Symbol Selector", &show_symbol_selector_);
    
    ImGui::Text("Available Symbols:");
    ImGui::Separator();
    
    for (const auto& [symbol, data] : symbol_data_) {
        bool is_current = (symbol == current_symbol_);
        if (ImGui::Selectable(symbol.c_str(), is_current)) {
            current_symbol_ = symbol;
        }
        
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Data points: %zu", data.size());
        }
    }
    
    ImGui::End();
}

void VisualizationManager::renderFeatureDashboard() {
    ImGui::Begin("Feature Dashboard", &show_feature_dashboard_);
    
    if (!current_symbol_.empty() && hasDataForSymbol(current_symbol_)) {
        const auto& data = getCurrentData();
        renderer_.renderCategoryDashboard(data, current_symbol_);
    } else {
        ImGui::Text("No data available for current symbol");
    }
    
    ImGui::End();
}

void VisualizationManager::renderComparisonDashboard() {
    ImGui::Begin("Comparison Dashboard", &show_comparison_dashboard_);
    
    // Symbol selection for comparison
    ImGui::Text("Select symbols to compare:");
    
    for (const auto& [symbol, data] : symbol_data_) {
        bool selected = std::find(selected_symbols_for_comparison_.begin(), 
                                selected_symbols_for_comparison_.end(), symbol) 
                       != selected_symbols_for_comparison_.end();
        
        if (ImGui::Checkbox(symbol.c_str(), &selected)) {
            if (selected) {
                selected_symbols_for_comparison_.push_back(symbol);
            } else {
                selected_symbols_for_comparison_.erase(
                    std::remove(selected_symbols_for_comparison_.begin(),
                              selected_symbols_for_comparison_.end(), symbol),
                    selected_symbols_for_comparison_.end());
            }
        }
    }
    
    if (selected_symbols_for_comparison_.size() >= 2) {
        ImGui::Separator();
        ImGui::Text("Comparison Charts:");
        
        // Create comparison charts for selected symbols
        // This is a simplified version - you could expand this significantly
        for (const auto& symbol : selected_symbols_for_comparison_) {
            if (hasDataForSymbol(symbol)) {
                const auto& data = symbol_data_.at(symbol);
                auto price_chart = ChartFactory::createPriceChart(data, symbol);
                ChartFactory::renderChart(price_chart);
            }
        }
    }
    
    ImGui::End();
}

void VisualizationManager::renderStatisticsDashboard() {
    ImGui::Begin("Statistics Dashboard", &show_statistics_dashboard_);
    
    if (!current_symbol_.empty() && hasDataForSymbol(current_symbol_)) {
        const auto& data = getCurrentData();
        renderer_.renderStatisticalSummary(data, current_symbol_);
    } else {
        ImGui::Text("No data available for current symbol");
    }
    
    ImGui::End();
}

void VisualizationManager::addFeature(const std::string& name, 
                                     const std::string& display_name,
                                     FeatureCategory category, 
                                     ChartType chart_type) {
    auto& registry = FeatureRegistry::getInstance();
    registry.registerFeature(name, display_name, category, chart_type);
}

std::vector<std::string> VisualizationManager::getAvailableSymbols() const {
    std::vector<std::string> symbols;
    for (const auto& [symbol, data] : symbol_data_) {
        symbols.push_back(symbol);
    }
    return symbols;
}

std::vector<std::string> VisualizationManager::getAvailableFeatures() const {
    if (!current_symbol_.empty() && hasDataForSymbol(current_symbol_)) {
        return renderer_.getAvailableFeatures(getCurrentData());
    }
    return {};
}

std::vector<FeatureCategory> VisualizationManager::getAvailableCategories() const {
    if (!current_symbol_.empty() && hasDataForSymbol(current_symbol_)) {
        return renderer_.getAvailableCategories(getCurrentData());
    }
    return {};
}

void VisualizationManager::setDashboardLayout(const DashboardLayout& layout) {
    renderer_.setDashboardLayout(layout);
}

void VisualizationManager::setChartFilter(const ChartFilter& filter) {
    renderer_.setChartFilter(filter);
}

bool VisualizationManager::hasDataForSymbol(const std::string& symbol) const {
    return symbol_data_.find(symbol) != symbol_data_.end();
}

size_t VisualizationManager::getDataPointCount(const std::string& symbol) const {
    std::string target_symbol = symbol.empty() ? current_symbol_ : symbol;
    
    if (hasDataForSymbol(target_symbol)) {
        return symbol_data_.at(target_symbol).size();
    }
    return 0;
}

void VisualizationManager::exportChartAsImage(const std::string& feature_name, 
                                            const std::string& output_path) {
    // This would require additional ImGui/ImPlot functionality for image export
    // For now, just show a message
    showInfoMessage("Chart export functionality coming soon!");
}

void VisualizationManager::exportDashboardAsImages(const std::string& output_directory) {
    // This would require additional ImGui/ImPlot functionality for image export
    showInfoMessage("Dashboard export functionality coming soon!");
}

void VisualizationManager::loadDemoData() {
    // Create some demo data for testing
    std::vector<FlexibleStockData> demo_data;
    
    for (int i = 0; i < 100; ++i) {
        FlexibleStockData point;
        point.symbol = "DEMO";
        point.datetime_index = static_cast<float>(i);
        point.date_string = "2024-01-" + std::to_string(i % 30 + 1);
        
        // Generate some demo features
        double base_price = 100.0 + 10.0 * std::sin(i * 0.1);
        point.setFeature("close", base_price);
        point.setFeature("open", base_price + (rand() % 200 - 100) / 100.0);
        point.setFeature("high", base_price + (rand() % 300) / 100.0);
        point.setFeature("low", base_price - (rand() % 300) / 100.0);
        point.setFeature("volume", 1000000 + rand() % 500000);
        point.setFeature("rsi", 30 + (rand() % 40));
        point.setFeature("sma", base_price + (rand() % 200 - 100) / 200.0);
        
        demo_data.push_back(point);
    }
    
    symbol_data_["DEMO"] = std::move(demo_data);
    current_symbol_ = "DEMO";
    
    showInfoMessage("Demo data loaded successfully!");
}

void VisualizationManager::createExampleDashboard() {
    // Set up an example dashboard configuration
    DashboardLayout layout(DashboardLayout::LayoutType::TABS, 2, 2);
    setDashboardLayout(layout);
    
    ChartFilter filter;
    filter.show_price_charts = true;
    filter.show_volume_charts = true;
    filter.show_technical_indicators = true;
    setChartFilter(filter);
}

// Private helper methods
const std::vector<FlexibleStockData>& VisualizationManager::getCurrentData() const {
    static std::vector<FlexibleStockData> empty_data;
    
    if (current_symbol_.empty() || !hasDataForSymbol(current_symbol_)) {
        return empty_data;
    }
    
    return symbol_data_.at(current_symbol_);
}

std::vector<FlexibleStockData>& VisualizationManager::getCurrentData() {
    static std::vector<FlexibleStockData> empty_data;
    
    if (current_symbol_.empty() || !hasDataForSymbol(current_symbol_)) {
        return empty_data;
    }
    
    return symbol_data_[current_symbol_];
}

void VisualizationManager::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load CSV...")) {
                // This would open a file dialog in a real implementation
                showInfoMessage("File dialog not implemented yet");
            }
            if (ImGui::MenuItem("Load Demo Data")) {
                loadDemoData();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Handle exit
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Symbol Selector", nullptr, &show_symbol_selector_);
            ImGui::MenuItem("Feature Dashboard", nullptr, &show_feature_dashboard_);
            ImGui::MenuItem("Comparison Dashboard", nullptr, &show_comparison_dashboard_);
            ImGui::MenuItem("Statistics Dashboard", nullptr, &show_statistics_dashboard_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Create Example Dashboard")) {
                createExampleDashboard();
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void VisualizationManager::renderStatusBar() {
    ImGui::Begin("Status", nullptr, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Current Symbol: %s | Data Points: %zu | Features: %zu", 
               current_symbol_.c_str(),
               getDataPointCount(),
               getAvailableFeatures().size());
    
    ImGui::End();
}

void VisualizationManager::renderDataInfo() {
    if (ImGui::CollapsingHeader("Data Information")) {
        renderer_.renderDataStatistics(getCurrentData());
    }
}

void VisualizationManager::renderSymbolCombo() {
    ImGui::Text("Symbol:");
    ImGui::SameLine();
    
    if (ImGui::BeginCombo("##symbol", current_symbol_.c_str())) {
        for (const auto& [symbol, data] : symbol_data_) {
            bool is_selected = (symbol == current_symbol_);
            if (ImGui::Selectable(symbol.c_str(), is_selected)) {
                current_symbol_ = symbol;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void VisualizationManager::renderFeatureSelectionUI() {
    // This could be expanded for more sophisticated feature selection
    ImGui::Text("Feature Selection UI - Coming Soon");
}

void VisualizationManager::renderLayoutSelectionUI() {
    // This could be expanded for layout customization
    ImGui::Text("Layout Selection UI - Coming Soon");
}

void VisualizationManager::showErrorMessage(const std::string& message) {
    last_error_message_ = message;
    show_error_popup_ = true;
}

void VisualizationManager::showInfoMessage(const std::string& message) {
    last_info_message_ = message;
    show_info_popup_ = true;
}

// EasyViz convenience functions
namespace EasyViz {
    static VisualizationManager* g_manager = nullptr;
    
    void ensureManager() {
        if (!g_manager) {
            g_manager = new VisualizationManager();
            g_manager->initialize();
        }
    }
    
    void visualizeCSV(const std::string& csv_path) {
        ensureManager();
        g_manager->loadData(csv_path);
        // In a real implementation, this would start the main loop
    }
    
    void visualizeFeatureSet(const std::string& symbol, 
                           const OHLCVData& ohlcv_data, 
                           const FeatureSet& feature_set) {
        ensureManager();
        g_manager->loadData(symbol, ohlcv_data, feature_set);
        // In a real implementation, this would start the main loop
    }
    
    void compareFeatures(const std::vector<std::string>& feature_names,
                        const std::string& csv_path) {
        ensureManager();
        g_manager->loadData(csv_path);
        // Set up comparison view
    }
    
    void visualizeCategory(FeatureCategory category, const std::string& csv_path) {
        ensureManager();
        g_manager->loadData(csv_path);
        // Set up category view
    }
    
    void showStatistics(const std::string& csv_path) {
        ensureManager();
        g_manager->loadData(csv_path);
        // Show statistics view
    }
}

} // namespace Visualization
