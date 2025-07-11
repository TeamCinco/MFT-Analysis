#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

std::vector<std::string> FileManager::getCSVFiles(const std::string& directory) {
    std::vector<std::string> csvFiles;
    DIR* dir = opendir(directory.c_str());
    
    if (dir == nullptr) {
        return csvFiles;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Check if file ends with _features.csv
        if (filename.length() > 13 && 
            filename.substr(filename.length() - 13) == "_features.csv") {
            csvFiles.push_back(directory + "/" + filename);
        }
    }
    
    closedir(dir);
    std::sort(csvFiles.begin(), csvFiles.end());
    return csvFiles;
}

bool FileManager::loadCSVData(const std::string& filename, 
                             std::map<std::string, std::vector<StockData>>& stockDataMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Extract symbol from filename (remove path and _features.csv)
    std::string symbolFromFile = filename;
    size_t lastSlash = symbolFromFile.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        symbolFromFile = symbolFromFile.substr(lastSlash + 1);
    }
    size_t featuresPos = symbolFromFile.find("_features.csv");
    if (featuresPos != std::string::npos) {
        symbolFromFile = symbolFromFile.substr(0, featuresPos);
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int rowCount = 0;
    float dateIndex = 0;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;
        
        // Parse CSV with comma separation
        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        
        if (row.size() >= 29) { // All columns present
            try {
                StockData data;
                
                data.datetime_index = dateIndex++;
                data.date_string = row[0];
                data.open = std::stof(row[1]);
                data.high = std::stof(row[2]);
                data.low = std::stof(row[3]);
                data.close = std::stof(row[4]);
                data.volume = std::stof(row[5]);
                
                // Use symbol from filename, not from CSV (more reliable)
                data.symbol = symbolFromFile;
                
                // Parse all the technical indicators
                data.returns = std::stof(row[8]);
                data.sma = std::stof(row[9]);
                data.rsi = std::stof(row[10]);
                data.volatility = std::stof(row[11]);
                data.momentum = std::stof(row[12]);
                data.spread = std::stof(row[13]);
                data.internal_bar_strength = std::stof(row[14]);
                data.skewness_30 = std::stof(row[15]);
                data.kurtosis_30 = std::stof(row[16]);
                data.log_pct_change_5 = std::stof(row[17]);
                data.auto_correlation_50_10 = std::stof(row[18]);
                data.kama_10_2_30 = std::stof(row[19]);
                data.linear_slope_20 = std::stof(row[20]);
                data.linear_slope_60 = std::stof(row[21]);
                data.parkinson_volatility_20 = std::stof(row[22]);
                data.volume_sma_20 = std::stof(row[23]);
                data.velocity = std::stof(row[24]);
                data.acceleration = std::stof(row[25]);
                data.candle_way = std::stof(row[26]);
                data.candle_filling = std::stof(row[27]);
                data.candle_amplitude = std::stof(row[28]);
                
                stockDataMap[data.symbol].push_back(data);
                rowCount++;
            } catch (const std::exception& e) {
                continue; // Skip invalid rows
            }
        }
    }
    
    return rowCount > 0;
}

void FileManager::loadAllCSVFiles(std::map<std::string, std::vector<StockData>>& stockDataMap,
                                 std::vector<std::string>& symbols,
                                 bool& isLoading,
                                 int& totalFilesFound,
                                 int& filesLoaded,
                                 std::string& loadingStatus) {
    isLoading = true;
    loadingStatus = "Scanning for CSV files...";
    stockDataMap.clear();
    symbols.clear();
    filesLoaded = 0;
    
    // Look for CSV files in data directory
    std::vector<std::string> csvFiles = getCSVFiles("data");
    
    // If no files in data directory, try current directory
    if (csvFiles.empty()) {
        csvFiles = getCSVFiles(".");
    }
    
    totalFilesFound = csvFiles.size();
    
    if (csvFiles.empty()) {
        loadingStatus = "No *_features.csv files found!";
        isLoading = false;
        return;
    }
    
    std::cout << "Found " << totalFilesFound << " CSV files to load..." << std::endl;
    
    // Load files with progress tracking
    for (const auto& file : csvFiles) {
        loadingStatus = "Loading: " + file.substr(file.find_last_of("/\\") + 1);
        
        if (loadCSVData(file, stockDataMap)) {
            filesLoaded++;
        }
        
        // Update progress every 100 files for performance
        if (filesLoaded % 100 == 0) {
            std::cout << "Loaded " << filesLoaded << "/" << totalFilesFound << " files..." << std::endl;
        }
    }
    
    // Extract unique symbols and sort
    symbols.clear();
    for (const auto& pair : stockDataMap) {
        symbols.push_back(pair.first);
    }
    std::sort(symbols.begin(), symbols.end());
    
    int totalDataPoints = 0;
    for (const auto& pair : stockDataMap) {
        totalDataPoints += pair.second.size();
    }
    
    std::cout << "Loading complete!" << std::endl;
    std::cout << "Total symbols: " << symbols.size() << std::endl;
    std::cout << "Total data points: " << totalDataPoints << std::endl;
    
    loadingStatus = "Complete! Loaded " + std::to_string(symbols.size()) + " symbols";
    isLoading = false;
}