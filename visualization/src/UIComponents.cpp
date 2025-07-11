#include "UIComponents.h"
#include "StockData.h"
#include "imgui.h"
#include <algorithm>

void UIComponents::renderLoadingProgress(bool isLoading, int filesLoaded, int totalFilesFound, 
                                        const std::string& loadingStatus) {
    if (isLoading) {
        ImGui::Text("Loading... %d/%d files", filesLoaded, totalFilesFound);
        ImGui::Text("%s", loadingStatus.c_str());
        
        if (totalFilesFound > 0) {
            float progress = (float)filesLoaded / (float)totalFilesFound;
            ImGui::ProgressBar(progress, ImVec2(400, 0));
        }
    } else if (!loadingStatus.empty()) {
        ImGui::Text("%s", loadingStatus.c_str());
    }
}

void UIComponents::renderDataSummary(const std::vector<std::string>& symbols, int totalDataPoints) {
    ImGui::Text("Total symbols loaded: %d", (int)symbols.size());
    ImGui::Text("Total data points: %d", totalDataPoints);
}

void UIComponents::renderSymbolSelector(const std::vector<std::string>& symbols, 
                                       int& selectedSymbol,
                                       const std::map<std::string, std::vector<StockData>>& stockDataMap) {
    // Symbol search/filter
    static char symbolFilter[64] = "";
    ImGui::InputText("Filter symbols", symbolFilter, sizeof(symbolFilter));
    
    // Create filtered symbol list
    std::vector<int> filteredIndices;
    std::string filterStr = symbolFilter;
    std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), ::toupper);
    
    for (int i = 0; i < symbols.size(); i++) {
        std::string symbolUpper = symbols[i];
        std::transform(symbolUpper.begin(), symbolUpper.end(), symbolUpper.begin(), ::toupper);
        
        if (filterStr.empty() || symbolUpper.find(filterStr) != std::string::npos) {
            filteredIndices.push_back(i);
        }
    }
    
    // Symbol dropdown with filtering
    const char* combo_preview = selectedSymbol < symbols.size() ? symbols[selectedSymbol].c_str() : "Select Symbol";
    if (ImGui::BeginCombo("Select Symbol", combo_preview)) {
        for (int idx : filteredIndices) {
            bool isSelected = (selectedSymbol == idx);
            auto it = stockDataMap.find(symbols[idx]);
            int dataCount = (it != stockDataMap.end()) ? it->second.size() : 0;
            std::string displayText = symbols[idx] + " (" + std::to_string(dataCount) + " points)";
            
            if (ImGui::Selectable(displayText.c_str(), isSelected)) {
                selectedSymbol = idx;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void UIComponents::renderSymbolInfo(const std::string& symbol, const std::vector<StockData>& data) {
    ImGui::Text("Symbol: %s", symbol.c_str());
    ImGui::Text("Data points: %d", (int)data.size());
    
    if (!data.empty()) {
        ImGui::Text("Date range: %s to %s", data.front().date_string.c_str(), data.back().date_string.c_str());
        ImGui::Text("Latest Close: $%.2f", data.back().close);
        ImGui::Text("Latest Volume: %.0f", data.back().volume);
        ImGui::Text("Latest RSI: %.2f", data.back().rsi);
        ImGui::Text("Latest Returns: %.4f", data.back().returns);
    }
}