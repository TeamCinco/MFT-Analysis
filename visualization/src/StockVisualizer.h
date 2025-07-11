#pragma once
#include <map>
#include <vector>
#include <string>
#include "StockData.h"

class StockVisualizer {
private:
    std::map<std::string, std::vector<StockData>> stockDataMap;
    std::vector<std::string> symbols;
    int selectedSymbol = 0;
    bool dataLoaded = false;
    bool isLoading = false;
    int totalFilesFound = 0;
    int filesLoaded = 0;
    std::string loadingStatus = "";
    
public:
    void loadAllCSVFiles();
    void renderUI();
    void renderCharts(const std::string& symbol, const std::vector<StockData>& data);
    int getTotalDataPoints();
    void clearData();
};