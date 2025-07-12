#include "StockVisualizer.h"
#include "FileManager.h"
#include "ChartRenderer.h"
#include "UIComponents.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>

void StockVisualizer::loadAllCSVFiles() {
    FileManager::loadAllCSVFiles(stockDataMap, symbols, isLoading, 
                                totalFilesFound, filesLoaded, loadingStatus);
    dataLoaded = !stockDataMap.empty();
}

void StockVisualizer::clearData() {
    stockDataMap.clear();
    symbols.clear();
    selectedSymbol = 0;
    dataLoaded = false;
    isLoading = false;
    loadingStatus = "";
}

int StockVisualizer::getTotalDataPoints() {
    int total = 0;
    for (const auto& pair : stockDataMap) {
        total += pair.second.size();
    }
    return total;
}

void StockVisualizer::renderUI() {
    ImGui::Begin("Stock Data Visualizer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    if (ImGui::Button("Load All Stock Data") && !isLoading) {
        loadAllCSVFiles();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear Data")) {
        clearData();
    }
    
    // Show loading status
    UIComponents::renderLoadingProgress(isLoading, filesLoaded, totalFilesFound, loadingStatus);
    
    if (dataLoaded && !symbols.empty()) {
        ImGui::Separator();
        UIComponents::renderDataSummary(symbols, getTotalDataPoints());
        UIComponents::renderSymbolSelector(symbols, selectedSymbol, stockDataMap);
        
        // Display selected symbol info
        if (selectedSymbol < symbols.size()) {
            const std::string& symbol = symbols[selectedSymbol];
            const auto& data = stockDataMap[symbol];
            
            UIComponents::renderSymbolInfo(symbol, data);
            ImGui::Separator();
            renderCharts(symbol, data);
        }
    } else if (!isLoading) {
        ImGui::Text("No data loaded.");
        ImGui::Text("Click 'Load All Stock Data' to load all *_features.csv files.");
        ImGui::Text("Expected file format: SYMBOL_features.csv");
    }
    
    ImGui::End();
}

void StockVisualizer::renderCharts(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    // Create tabs for different chart categories
    if (ImGui::BeginTabBar("ChartTabs")) {
        
        // Price & Volume Tab
        if (ImGui::BeginTabItem("Price & Volume")) {
            ChartRenderer::renderPriceVolumeCharts(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Technical Indicators Tab
        if (ImGui::BeginTabItem("Technical Indicators")) {
            ChartRenderer::renderTechnicalIndicators(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Advanced Features Tab
        if (ImGui::BeginTabItem("Advanced Features")) {
            ChartRenderer::renderAdvancedFeatures(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Oscillators Tab
        if (ImGui::BeginTabItem("Oscillators")) {
            ChartRenderer::renderOscillators(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Ichimoku & SuperTrend Tab
        if (ImGui::BeginTabItem("Ichimoku & Trends")) {
            ChartRenderer::renderIchimokuCloud(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Volume Profile Tab
        if (ImGui::BeginTabItem("Volume Profile")) {
            ChartRenderer::renderVolumeProfile(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Statistical Measures Tab
        if (ImGui::BeginTabItem("Statistical Measures")) {
            ChartRenderer::renderStatisticalMeasures(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Risk Metrics Tab
        if (ImGui::BeginTabItem("Risk Metrics")) {
            ChartRenderer::renderRiskMetrics(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Regime Analysis Tab
        if (ImGui::BeginTabItem("Regime Analysis")) {
            ChartRenderer::renderRegimeAnalysis(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Distribution & Shape Tab
        if (ImGui::BeginTabItem("Distribution & Shape")) {
            ChartRenderer::renderDistributionShapeCharts(symbol, data);
            ImGui::EndTabItem();
        }
        
        // Summary Statistics Tab
        if (ImGui::BeginTabItem("Statistics")) {
            ChartRenderer::renderStatistics(data);
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}
