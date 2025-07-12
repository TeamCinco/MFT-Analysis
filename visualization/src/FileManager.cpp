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
    int totalFilesScanned = 0;
    const int MAX_FILES_TO_SCAN = 20000; // Safety limit to prevent infinite scanning
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Skip . and .. entries
        if (filename == "." || filename == "..") {
            continue;
        }
        
        totalFilesScanned++;
        
        // Safety check: if we're scanning too many files, we're probably in the wrong directory
        if (totalFilesScanned > MAX_FILES_TO_SCAN) {
            std::cout << "WARNING: Scanned " << totalFilesScanned << " files in " << directory 
                      << " - stopping to prevent infinite loop" << std::endl;
            break;
        }
        
        // Check if file ends with _features.csv
        if (filename.length() > 13 && 
            filename.substr(filename.length() - 13) == "_features.csv") {
            csvFiles.push_back(directory + "/" + filename);
        }
        
        // Progress indicator for large directories
        if (totalFilesScanned % 5000 == 0) {
            std::cout << "  Scanned " << totalFilesScanned << " files, found " 
                      << csvFiles.size() << " CSV files so far..." << std::endl;
        }
    }
    
    closedir(dir);
    
    if (totalFilesScanned > 1000 && csvFiles.empty()) {
        std::cout << "  Scanned " << totalFilesScanned << " files but found no *_features.csv files" << std::endl;
    }
    
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
        
        if (row.size() >= 64) { // All columns present (updated for new features)
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
                data.data_frequency = row[7]; // data_frequency column
                
                // Helper function to safely parse float values
                auto safeStof = [](const std::string& str) -> float {
                    if (str.empty()) return 0.0f;
                    try {
                        return std::stof(str);
                    } catch (const std::exception&) {
                        return 0.0f;
                    }
                };
                
                // Parse all the technical indicators (columns 8-28) with safe conversion
                data.returns = safeStof(row[8]);
                data.sma = safeStof(row[9]);
                data.rsi = safeStof(row[10]);
                data.volatility = safeStof(row[11]);
                data.momentum = safeStof(row[12]);
                data.spread = safeStof(row[13]);
                data.internal_bar_strength = safeStof(row[14]);
                data.skewness_30 = safeStof(row[15]);
                data.kurtosis_30 = safeStof(row[16]);
                data.log_pct_change_5 = safeStof(row[17]);
                data.auto_correlation_50_10 = safeStof(row[18]);
                data.kama_10_2_30 = safeStof(row[19]);
                data.linear_slope_20 = safeStof(row[20]);
                data.linear_slope_60 = safeStof(row[21]);
                data.parkinson_volatility_20 = safeStof(row[22]);
                data.volume_sma_20 = safeStof(row[23]);
                data.velocity = safeStof(row[24]);
                data.acceleration = safeStof(row[25]);
                data.candle_way = safeStof(row[26]);
                data.candle_filling = safeStof(row[27]);
                data.candle_amplitude = safeStof(row[28]);
                
                // Parse new advanced features (columns 29-63) with safe conversion
                data.z_score_20 = safeStof(row[29]);
                data.percentile_rank_50 = safeStof(row[30]);
                data.coefficient_of_variation_30 = safeStof(row[31]);
                data.detrended_price_oscillator_20 = safeStof(row[32]);
                data.hurst_exponent_100 = safeStof(row[33]);
                data.garch_volatility_21 = safeStof(row[34]);
                data.shannon_entropy_volume_10 = safeStof(row[35]);
                data.chande_momentum_oscillator_14 = safeStof(row[36]);
                data.aroon_oscillator_25 = safeStof(row[37]);
                data.trix_15 = safeStof(row[38]);
                data.vortex_indicator_14 = safeStof(row[39]);
                data.supertrend_10_3 = safeStof(row[40]);
                data.ichimoku_senkou_span_A_9_26 = safeStof(row[41]);
                data.ichimoku_senkou_span_B_26_52 = safeStof(row[42]);
                data.fisher_transform_10 = safeStof(row[43]);
                data.volume_weighted_average_price_intraday = safeStof(row[44]);
                data.volume_profile_high_volume_node_intraday = safeStof(row[45]);
                data.volume_profile_low_volume_node_intraday = safeStof(row[46]);
                data.on_balance_volume_sma_20 = safeStof(row[47]);
                data.klinger_oscillator_34_55 = safeStof(row[48]);
                data.money_flow_index_14 = safeStof(row[49]);
                data.vwap_deviation_stddev_30 = safeStof(row[50]);
                data.markov_regime_switching_garch_2_state = safeStof(row[51]);
                data.adx_rating_14 = safeStof(row[52]);
                data.chow_test_statistic_breakpoint_detection_50 = safeStof(row[53]);
                data.market_regime_hmm_3_states_price_vol = safeStof(row[54]);
                data.high_volatility_indicator_garch_threshold = safeStof(row[55]);
                data.return_x_volume_interaction_10 = safeStof(row[56]);
                data.volatility_x_rsi_interaction_14 = safeStof(row[57]);
                data.price_to_kama_ratio_20_10_30 = safeStof(row[58]);
                data.polynomial_regression_price_degree_2_slope = safeStof(row[59]);
                data.conditional_value_at_risk_cvar_95_20 = safeStof(row[60]);
                data.drawdown_duration_from_peak_50 = safeStof(row[61]);
                data.ulcer_index_14 = safeStof(row[62]);
                data.sortino_ratio_30 = safeStof(row[63]);
                
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
    
    // Try multiple possible data directory locations
    std::vector<std::string> possiblePaths = {
        "visualization/data",           // From project root
        "./visualization/data",         // From project root with explicit current dir
        "../visualization/data",        // From build directory
        "data",                        // Legacy path
        "./data"                       // Legacy path with explicit current dir
    };
    
    std::vector<std::string> csvFiles;
    std::string foundPath;
    
    std::cout << "Searching for data directory..." << std::endl;
    
    for (const auto& path : possiblePaths) {
        std::cout << "  Checking: " << path << std::endl;
        csvFiles = getCSVFiles(path);
        
        if (!csvFiles.empty()) {
            foundPath = path;
            std::cout << "  ✓ Found " << csvFiles.size() << " CSV files in: " << path << std::endl;
            break;
        } else {
            std::cout << "  ✗ No CSV files found in: " << path << std::endl;
        }
    }
    
    // Safety check: if we found too many files, we might be scanning the wrong directory
    if (csvFiles.size() > 15000) {
        std::cout << "WARNING: Found " << csvFiles.size() << " files - this seems too many!" << std::endl;
        std::cout << "This might indicate scanning the wrong directory. Aborting to prevent infinite loop." << std::endl;
        loadingStatus = "Error: Too many files found - check data directory path";
        isLoading = false;
        return;
    }
    
    totalFilesFound = csvFiles.size();
    
    if (csvFiles.empty()) {
        std::cout << "No *_features.csv files found in any of the expected locations!" << std::endl;
        std::cout << "Expected locations checked:" << std::endl;
        for (const auto& path : possiblePaths) {
            std::cout << "  - " << path << std::endl;
        }
        loadingStatus = "No *_features.csv files found in expected locations!";
        isLoading = false;
        return;
    }
    
    std::cout << "Found " << totalFilesFound << " CSV files to load from: " << foundPath << std::endl;
    
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
