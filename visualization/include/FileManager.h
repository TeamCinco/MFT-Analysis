#pragma once
#include <string>
#include <vector>
#include <map>
#include "StockData.h"

class FileManager {
public:
    static std::vector<std::string> getCSVFiles(const std::string& directory);
    static bool loadCSVData(const std::string& filename, 
                           std::map<std::string, std::vector<StockData>>& stockDataMap);
    static void loadAllCSVFiles(std::map<std::string, std::vector<StockData>>& stockDataMap,
                               std::vector<std::string>& symbols,
                               bool& isLoading,
                               int& totalFilesFound,
                               int& filesLoaded,
                               std::string& loadingStatus);
};