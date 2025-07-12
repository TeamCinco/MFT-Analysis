#pragma once
#include <vector>
#include <string>
#include <map>

// Forward declaration
struct StockData;

class UIComponents {
public:
    static void renderLoadingProgress(bool isLoading, int filesLoaded, int totalFilesFound, 
                                     const std::string& loadingStatus);
    
    static void renderSymbolSelector(const std::vector<std::string>& symbols, 
                                    int& selectedSymbol,
                                    const std::map<std::string, std::vector<StockData>>& stockDataMap);
    
    static void renderDataSummary(const std::vector<std::string>& symbols, 
                                 int totalDataPoints);
    
    static void renderSymbolInfo(const std::string& symbol, 
                                const std::vector<StockData>& data);
};