#include "CointegrationVisualizer.h"
#include "imgui.h"
#include "implot.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

void CointegrationVisualizer::loadCSVFile(const std::string& filename) {
    isLoading = true;
    loadingStatus = "Loading " + filename + "...";
    pairsData.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        loadingStatus = "Error: Could not open file " + filename;
        isLoading = false;
        return;
    }
    
    std::string line;
    bool isFirstLine = true;
    
    while (std::getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue; // Skip header
        }
        
        std::stringstream ss(line);
        std::string cell;
        CointegrationData data;
        
        try {
            // Parse CSV line
            std::getline(ss, data.stock1, ',');
            std::getline(ss, data.stock2, ',');
            
            std::getline(ss, cell, ','); data.adf_statistic = std::stof(cell);
            std::getline(ss, cell, ','); data.p_value = std::stof(cell);
            std::getline(ss, cell, ','); data.critical_1pct = std::stof(cell);
            std::getline(ss, cell, ','); data.critical_5pct = std::stof(cell);
            std::getline(ss, cell, ','); data.critical_10pct = std::stof(cell);
            std::getline(ss, cell, ','); data.half_life_days = std::stof(cell);
            std::getline(ss, cell, ','); data.hedge_ratio = std::stof(cell);
            std::getline(ss, cell, ','); data.spread_mean = std::stof(cell);
            std::getline(ss, cell, ','); data.spread_stddev = std::stof(cell);
            std::getline(ss, cell, ','); data.max_spread = std::stof(cell);
            std::getline(ss, cell, ','); data.min_spread = std::stof(cell);
            std::getline(ss, cell, ','); data.current_spread = std::stof(cell);
            std::getline(ss, cell, ','); data.z_score = std::stof(cell);
            std::getline(ss, data.grade, ',');
            
            std::getline(ss, cell, ','); 
            data.is_cointegrated = (cell == "TRUE" || cell == "true" || cell == "1");
            
            std::getline(ss, cell, ','); data.entry_threshold = std::stof(cell);
            std::getline(ss, cell, ','); data.exit_threshold = std::stof(cell);
            std::getline(ss, cell, ','); data.expected_return = std::stof(cell);
            std::getline(ss, cell, ','); data.sharpe_ratio = std::stof(cell);
            std::getline(ss, cell, ','); data.historical_trades = std::stoi(cell);
            std::getline(ss, cell, ','); data.win_rate = std::stof(cell);
            
            pairsData.push_back(data);
            
        } catch (const std::exception& e) {
            std::cerr << "Error parsing line: " << line << " - " << e.what() << std::endl;
            continue;
        }
    }
    
    file.close();
    dataLoaded = true;
    isLoading = false;
    loadingStatus = "Loaded " + std::to_string(pairsData.size()) + " pairs";
    
    updateAnalysis();
    applyFilters();
}

void CointegrationVisualizer::renderUI() {
    ImGui::Begin("Cointegration Pairs Analysis", nullptr, ImGuiWindowFlags_MenuBar);
    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load CSV...")) {
                // For now, load the sample file
                loadCSVFile("cointegration_sample.csv");
            }
            if (ImGui::MenuItem("Export Filtered Data...")) {
                exportFilteredData("filtered_pairs.csv");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    if (isLoading) {
        ImGui::Text("%s", loadingStatus.c_str());
        ImGui::End();
        return;
    }
    
    if (!dataLoaded) {
        ImGui::Text("No data loaded. Use File -> Load CSV to load cointegration data.");
        ImGui::End();
        return;
    }
    
    // Create tabs for different views
    if (ImGui::BeginTabBar("AnalysisTabs")) {
        if (ImGui::BeginTabItem("Dashboard")) {
            renderDashboard();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Scatter Plots")) {
            renderScatterPlots();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Distributions")) {
            renderDistributions();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Top Opportunities")) {
            renderTopOpportunities();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Pair Details")) {
            renderPairDetails();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void CointegrationVisualizer::renderDashboard() {
    // Filters section
    renderFilters();
    
    ImGui::Separator();
    
    // Summary statistics
    ImGui::Text("Analysis Summary");
    ImGui::Columns(4, "SummaryColumns");
    
    ImGui::Text("Total Pairs: %d", analysisResults.totalPairs);
    ImGui::NextColumn();
    ImGui::Text("Cointegrated: %d", analysisResults.cointegrated);
    ImGui::NextColumn();
    ImGui::Text("High Quality: %d", analysisResults.highQuality);
    ImGui::NextColumn();
    ImGui::Text("Outliers: %d", analysisResults.outliers);
    ImGui::NextColumn();
    
    ImGui::Text("Avg Sharpe: %.3f", analysisResults.avgSharpeRatio);
    ImGui::NextColumn();
    ImGui::Text("Avg Win Rate: %.1f%%", analysisResults.avgWinRate * 100);
    ImGui::NextColumn();
    ImGui::Text("Avg Half-Life: %.1f days", analysisResults.avgHalfLife);
    ImGui::NextColumn();
    ImGui::Text("Status: %s", loadingStatus.c_str());
    
    ImGui::Columns(1);
    ImGui::Separator();
    
    // Quick overview charts
    if (ImPlot::BeginPlot("Risk vs Return Overview", ImVec2(-1, 300))) {
        ImPlot::SetupAxes("Sharpe Ratio", "Expected Return");
        
        auto filtered = getFilteredData();
        std::vector<float> x_data, y_data;
        for (const auto& pair : filtered) {
            x_data.push_back(pair.sharpe_ratio);
            y_data.push_back(pair.expected_return);
        }
        
        if (!x_data.empty()) {
            ImPlot::PlotScatter("Pairs", x_data.data(), y_data.data(), x_data.size());
        }
        
        ImPlot::EndPlot();
    }
}

void CointegrationVisualizer::renderScatterPlots() {
    auto filtered = getFilteredData();
    
    if (filtered.empty()) {
        ImGui::Text("No data matches current filters.");
        return;
    }
    
    // Prepare data vectors
    std::vector<float> adf_data, p_data, half_life_data, sharpe_data, return_data, z_data;
    
    for (const auto& pair : filtered) {
        adf_data.push_back(pair.adf_statistic);
        p_data.push_back(pair.p_value);
        half_life_data.push_back(pair.half_life_days);
        sharpe_data.push_back(pair.sharpe_ratio);
        return_data.push_back(pair.expected_return);
        z_data.push_back(pair.z_score);
    }
    
    // ADF vs P-Value plot
    if (ImPlot::BeginPlot("Statistical Significance", ImVec2(-1, 250))) {
        ImPlot::SetupAxes("ADF Statistic", "P-Value");
        ImPlot::PlotScatter("Pairs", adf_data.data(), p_data.data(), adf_data.size());
        
        // Add significance threshold line
        float sig_line_x[] = {-6, 0};
        float sig_line_y[] = {0.05f, 0.05f};
        ImPlot::PlotLine("5% Significance", sig_line_x, sig_line_y, 2);
        
        ImPlot::EndPlot();
    }
    
    // Risk vs Return plot
    if (ImPlot::BeginPlot("Risk vs Return", ImVec2(-1, 250))) {
        ImPlot::SetupAxes("Sharpe Ratio", "Expected Return");
        ImPlot::PlotScatter("Pairs", sharpe_data.data(), return_data.data(), sharpe_data.size());
        ImPlot::EndPlot();
    }
    
    // Half-Life vs Z-Score plot
    if (ImPlot::BeginPlot("Mean Reversion Analysis", ImVec2(-1, 250))) {
        ImPlot::SetupAxes("Half-Life (Days)", "Current Z-Score");
        ImPlot::PlotScatter("Pairs", half_life_data.data(), z_data.data(), half_life_data.size());
        
        // Add entry/exit threshold lines
        float entry_line_x[] = {0, 200};
        float entry_pos[] = {2.0f, 2.0f};
        float entry_neg[] = {-2.0f, -2.0f};
        ImPlot::PlotLine("Entry Threshold +", entry_line_x, entry_pos, 2);
        ImPlot::PlotLine("Entry Threshold -", entry_line_x, entry_neg, 2);
        
        ImPlot::EndPlot();
    }
}

void CointegrationVisualizer::renderDistributions() {
    auto filtered = getFilteredData();
    
    if (filtered.empty()) {
        ImGui::Text("No data matches current filters.");
        return;
    }
    
    // Prepare histogram data
    std::vector<float> sharpe_data, win_rate_data, half_life_data;
    
    for (const auto& pair : filtered) {
        sharpe_data.push_back(pair.sharpe_ratio);
        win_rate_data.push_back(pair.win_rate);
        half_life_data.push_back(pair.half_life_days);
    }
    
    // Sharpe Ratio Distribution
    if (ImPlot::BeginPlot("Sharpe Ratio Distribution", ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Sharpe Ratio", "Frequency");
        ImPlot::PlotHistogram("Distribution", sharpe_data.data(), sharpe_data.size(), 20);
        ImPlot::EndPlot();
    }
    
    // Win Rate Distribution
    if (ImPlot::BeginPlot("Win Rate Distribution", ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Win Rate", "Frequency");
        ImPlot::PlotHistogram("Distribution", win_rate_data.data(), win_rate_data.size(), 20);
        ImPlot::EndPlot();
    }
    
    // Half-Life Distribution
    if (ImPlot::BeginPlot("Half-Life Distribution", ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Half-Life (Days)", "Frequency");
        ImPlot::PlotHistogram("Distribution", half_life_data.data(), half_life_data.size(), 20);
        ImPlot::EndPlot();
    }
}

void CointegrationVisualizer::renderTopOpportunities() {
    auto filtered = getFilteredData();
    
    // Sort by Sharpe ratio descending
    std::sort(filtered.begin(), filtered.end(), 
              [](const CointegrationData& a, const CointegrationData& b) {
                  return a.sharpe_ratio > b.sharpe_ratio;
              });
    
    ImGui::Text("Top Trading Opportunities (Sorted by Sharpe Ratio)");
    ImGui::Separator();
    
    if (ImGui::BeginTable("TopPairs", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Pair");
        ImGui::TableSetupColumn("Grade");
        ImGui::TableSetupColumn("Sharpe");
        ImGui::TableSetupColumn("Win Rate");
        ImGui::TableSetupColumn("Z-Score");
        ImGui::TableSetupColumn("Half-Life");
        ImGui::TableSetupColumn("Expected Return");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();
        
        for (size_t i = 0; i < std::min(filtered.size(), size_t(20)); ++i) {
            const auto& pair = filtered[i];
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", pair.getPairName().c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", pair.grade.c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", pair.sharpe_ratio);
            
            ImGui::TableNextColumn();
            ImGui::Text("%.1f%%", pair.win_rate * 100);
            
            ImGui::TableNextColumn();
            if (abs(pair.z_score) > 2.0f) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%.2f", pair.z_score);
            } else {
                ImGui::Text("%.2f", pair.z_score);
            }
            
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", pair.half_life_days);
            
            ImGui::TableNextColumn();
            ImGui::Text("%.4f", pair.expected_return);
            
            ImGui::TableNextColumn();
            if (pair.isOutlier()) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Outlier");
            } else if (pair.isHighQuality()) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "High Quality");
            } else {
                ImGui::Text("Normal");
            }
        }
        
        ImGui::EndTable();
    }
}

void CointegrationVisualizer::renderPairDetails() {
    auto filtered = getFilteredData();
    
    if (filtered.empty()) {
        ImGui::Text("No pairs match current filters.");
        return;
    }
    
    // Pair selection
    std::vector<const char*> pair_names;
    for (const auto& pair : filtered) {
        pair_names.push_back(pair.getPairName().c_str());
    }
    
    ImGui::Text("Select Pair for Detailed Analysis:");
    if (ImGui::Combo("##PairSelect", &selectedPair, pair_names.data(), pair_names.size())) {
        selectedPair = std::min(selectedPair, (int)filtered.size() - 1);
    }
    
    if (selectedPair >= 0 && selectedPair < filtered.size()) {
        const auto& pair = filtered[selectedPair];
        
        ImGui::Separator();
        ImGui::Text("Detailed Analysis: %s", pair.getPairName().c_str());
        
        ImGui::Columns(2, "DetailColumns");
        
        // Left column - Basic info
        ImGui::Text("Basic Information");
        ImGui::Separator();
        ImGui::Text("Stock 1: %s", pair.stock1.c_str());
        ImGui::Text("Stock 2: %s", pair.stock2.c_str());
        ImGui::Text("Grade: %s", pair.grade.c_str());
        ImGui::Text("Cointegrated: %s", pair.is_cointegrated ? "Yes" : "No");
        
        ImGui::Text("\nStatistical Tests");
        ImGui::Separator();
        ImGui::Text("ADF Statistic: %.4f", pair.adf_statistic);
        ImGui::Text("P-Value: %.6f", pair.p_value);
        ImGui::Text("Critical 1%%: %.4f", pair.critical_1pct);
        ImGui::Text("Critical 5%%: %.4f", pair.critical_5pct);
        ImGui::Text("Critical 10%%: %.4f", pair.critical_10pct);
        
        ImGui::NextColumn();
        
        // Right column - Trading info
        ImGui::Text("Trading Metrics");
        ImGui::Separator();
        ImGui::Text("Expected Return: %.6f", pair.expected_return);
        ImGui::Text("Sharpe Ratio: %.4f", pair.sharpe_ratio);
        ImGui::Text("Win Rate: %.1f%%", pair.win_rate * 100);
        ImGui::Text("Historical Trades: %d", pair.historical_trades);
        
        ImGui::Text("\nSpread Analysis");
        ImGui::Separator();
        ImGui::Text("Current Z-Score: %.4f", pair.z_score);
        ImGui::Text("Half-Life: %.2f days", pair.half_life_days);
        ImGui::Text("Hedge Ratio: %.6f", pair.hedge_ratio);
        ImGui::Text("Entry Threshold: %.2f", pair.entry_threshold);
        ImGui::Text("Exit Threshold: %.2f", pair.exit_threshold);
        
        ImGui::Text("\nSpread Statistics");
        ImGui::Separator();
        ImGui::Text("Mean: %.6f", pair.spread_mean);
        ImGui::Text("Std Dev: %.6f", pair.spread_stddev);
        ImGui::Text("Current: %.6f", pair.current_spread);
        ImGui::Text("Max: %.6f", pair.max_spread);
        ImGui::Text("Min: %.6f", pair.min_spread);
        
        ImGui::Columns(1);
    }
}

void CointegrationVisualizer::renderFilters() {
    ImGui::Text("Filters");
    ImGui::Separator();
    
    ImGui::Columns(4, "FilterColumns");
    
    ImGui::Checkbox("Only Cointegrated", &showOnlyCointegrated);
    ImGui::NextColumn();
    ImGui::Checkbox("Only High Quality", &showOnlyHighQuality);
    ImGui::NextColumn();
    ImGui::Checkbox("Only Outliers", &showOnlyOutliers);
    ImGui::NextColumn();
    
    // Grade filter
    const char* grades[] = {"All", "A", "B", "C"};
    static int grade_idx = 0;
    if (ImGui::Combo("Grade", &grade_idx, grades, 4)) {
        gradeFilter = grades[grade_idx];
    }
    
    ImGui::NextColumn();
    ImGui::SliderFloat("Min Sharpe", &minSharpeRatio, -2.0f, 5.0f);
    ImGui::NextColumn();
    ImGui::SliderFloat("Max Half-Life", &maxHalfLife, 1.0f, 500.0f);
    
    ImGui::Columns(1);
    
    if (ImGui::Button("Apply Filters")) {
        applyFilters();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Filters")) {
        showOnlyCointegrated = false;
        showOnlyHighQuality = false;
        showOnlyOutliers = false;
        gradeFilter = "All";
        minSharpeRatio = 0.0f;
        maxHalfLife = 1000.0f;
        applyFilters();
    }
}

void CointegrationVisualizer::updateAnalysis() {
    analysisResults.totalPairs = pairsData.size();
    analysisResults.cointegrated = 0;
    analysisResults.highQuality = 0;
    analysisResults.outliers = 0;
    
    float sumSharpe = 0, sumWinRate = 0, sumHalfLife = 0;
    
    for (const auto& pair : pairsData) {
        if (pair.is_cointegrated) analysisResults.cointegrated++;
        if (pair.isHighQuality()) analysisResults.highQuality++;
        if (pair.isOutlier()) analysisResults.outliers++;
        
        sumSharpe += pair.sharpe_ratio;
        sumWinRate += pair.win_rate;
        sumHalfLife += pair.half_life_days;
    }
    
    if (pairsData.size() > 0) {
        analysisResults.avgSharpeRatio = sumSharpe / pairsData.size();
        analysisResults.avgWinRate = sumWinRate / pairsData.size();
        analysisResults.avgHalfLife = sumHalfLife / pairsData.size();
    }
}

void CointegrationVisualizer::applyFilters() {
    // This function is called when filters change
    // The actual filtering is done in getFilteredData()
}

std::vector<CointegrationData> CointegrationVisualizer::getFilteredData() const {
    std::vector<CointegrationData> filtered;
    
    for (const auto& pair : pairsData) {
        // Apply filters
        if (showOnlyCointegrated && !pair.is_cointegrated) continue;
        if (showOnlyHighQuality && !pair.isHighQuality()) continue;
        if (showOnlyOutliers && !pair.isOutlier()) continue;
        if (gradeFilter != "All" && pair.grade != gradeFilter) continue;
        if (pair.sharpe_ratio < minSharpeRatio) continue;
        if (pair.half_life_days > maxHalfLife) continue;
        
        filtered.push_back(pair);
    }
    
    return filtered;
}

void CointegrationVisualizer::exportFilteredData(const std::string& filename) const {
    auto filtered = getFilteredData();
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file for export: " << filename << std::endl;
        return;
    }
    
    // Write header
    file << "Stock1,Stock2,ADF_Statistic,P_Value,Critical_1pct,Critical_5pct,Critical_10pct,";
    file << "Half_Life_Days,Hedge_Ratio,Spread_Mean,Spread_StdDev,Max_Spread,Min_Spread,";
    file << "Current_Spread,Z_Score,Grade,Is_Cointegrated,Entry_Threshold,Exit_Threshold,";
    file << "Expected_Return,Sharpe_Ratio,Historical_Trades,Win_Rate\n";
    
    // Write data
    for (const auto& pair : filtered) {
        file << pair.stock1 << "," << pair.stock2 << "," << pair.adf_statistic << ",";
        file << pair.p_value << "," << pair.critical_1pct << "," << pair.critical_5pct << ",";
        file << pair.critical_10pct << "," << pair.half_life_days << "," << pair.hedge_ratio << ",";
        file << pair.spread_mean << "," << pair.spread_stddev << "," << pair.max_spread << ",";
        file << pair.min_spread << "," << pair.current_spread << "," << pair.z_score << ",";
        file << pair.grade << "," << (pair.is_cointegrated ? "TRUE" : "FALSE") << ",";
        file << pair.entry_threshold << "," << pair.exit_threshold << "," << pair.expected_return << ",";
        file << pair.sharpe_ratio << "," << pair.historical_trades << "," << pair.win_rate << "\n";
    }
    
    file.close();
    std::cout << "Exported " << filtered.size() << " pairs to " << filename << std::endl;
}

void CointegrationVisualizer::clearData() {
    pairsData.clear();
    dataLoaded = false;
    selectedPair = 0;
}
