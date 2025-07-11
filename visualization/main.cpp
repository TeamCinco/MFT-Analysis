#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

struct StockData {
    float datetime_index;
    float open, high, low, close, volume;
    std::string symbol;
    float returns, sma, rsi, volatility, momentum;
    float spread, internal_bar_strength, skewness_30, kurtosis_30;
    float log_pct_change_5, auto_correlation_50_10, kama_10_2_30;
    float linear_slope_20, linear_slope_60, parkinson_volatility_20;
    float volume_sma_20, velocity, acceleration;
    float candle_way, candle_filling, candle_amplitude;
    std::string date_string;
};

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
    
    // Get list of CSV files in directory
    std::vector<std::string> getCSVFiles(const std::string& directory) {
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
    
public:
    bool LoadCSVData(const std::string& filename) {
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
    
    void LoadAllCSVFiles() {
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
            
            if (LoadCSVData(file)) {
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
        dataLoaded = true;
        isLoading = false;
    }
    
    void RenderUI() {
        ImGui::Begin("Stock Data Visualizer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        if (ImGui::Button("Load All Stock Data") && !isLoading) {
            LoadAllCSVFiles();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Clear Data")) {
            stockDataMap.clear();
            symbols.clear();
            selectedSymbol = 0;
            dataLoaded = false;
            isLoading = false;
        }
        
        // Show loading status
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
        
        if (dataLoaded && !symbols.empty()) {
            ImGui::Separator();
            ImGui::Text("Total symbols loaded: %d", (int)symbols.size());
            ImGui::Text("Total data points: %d", getTotalDataPoints());
            
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
                    std::string displayText = symbols[idx] + " (" + std::to_string(stockDataMap[symbols[idx]].size()) + " points)";
                    
                    if (ImGui::Selectable(displayText.c_str(), isSelected)) {
                        selectedSymbol = idx;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            
            // Display selected symbol info
            if (selectedSymbol < symbols.size()) {
                const std::string& symbol = symbols[selectedSymbol];
                const auto& data = stockDataMap[symbol];
                
                ImGui::Separator();
                ImGui::Text("Symbol: %s", symbol.c_str());
                ImGui::Text("Data points: %d", (int)data.size());
                
                if (!data.empty()) {
                    ImGui::Text("Date range: %s to %s", data.front().date_string.c_str(), data.back().date_string.c_str());
                    ImGui::Text("Latest Close: $%.2f", data.back().close);
                    ImGui::Text("Latest Volume: %.0f", data.back().volume);
                    ImGui::Text("Latest RSI: %.2f", data.back().rsi);
                    ImGui::Text("Latest Returns: %.4f", data.back().returns);
                }
                
                ImGui::Separator();
                RenderCharts(symbol, data);
            }
        } else if (!isLoading) {
            ImGui::Text("No data loaded.");
            ImGui::Text("Click 'Load All Stock Data' to load all *_features.csv files.");
            ImGui::Text("Expected file format: SYMBOL_features.csv");
        }
        
        ImGui::End();
    }
    
    int getTotalDataPoints() {
        int total = 0;
        for (const auto& pair : stockDataMap) {
            total += pair.second.size();
        }
        return total;
    }
    
    void RenderCharts(const std::string& symbol, const std::vector<StockData>& data) {
        if (data.empty()) return;
        
        // Prepare common data
        std::vector<float> indices;
        for (size_t i = 0; i < data.size(); ++i) {
            indices.push_back((float)i);
        }
        
        // Create tabs for different chart categories
        if (ImGui::BeginTabBar("ChartTabs")) {
            
            // Price & Volume Tab
            if (ImGui::BeginTabItem("Price & Volume")) {
                std::vector<float> opens, highs, lows, closes, volumes, sma_values, volume_sma_values;
                for (const auto& point : data) {
                    opens.push_back(point.open);
                    highs.push_back(point.high);
                    lows.push_back(point.low);
                    closes.push_back(point.close);
                    volumes.push_back(point.volume);
                    sma_values.push_back(point.sma);
                    volume_sma_values.push_back(point.volume_sma_20);
                }
                
                // OHLC Price Chart
                if (ImPlot::BeginPlot(("OHLC Price - " + symbol).c_str(), ImVec2(-1, 300))) {
                    ImPlot::SetupAxes("Time Index", "Price ($)");
                    ImPlot::PlotLine("Open", indices.data(), opens.data(), indices.size());
                    ImPlot::PlotLine("High", indices.data(), highs.data(), indices.size());
                    ImPlot::PlotLine("Low", indices.data(), lows.data(), indices.size());
                    ImPlot::PlotLine("Close", indices.data(), closes.data(), indices.size());
                    ImPlot::PlotLine("SMA", indices.data(), sma_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Volume Chart with SMA
                if (ImPlot::BeginPlot(("Volume Analysis - " + symbol).c_str(), ImVec2(-1, 200))) {
                    ImPlot::SetupAxes("Time Index", "Volume");
                    ImPlot::PlotBars("Volume", indices.data(), volumes.data(), indices.size(), 0.8);
                    ImPlot::PlotLine("Volume SMA 20", indices.data(), volume_sma_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                ImGui::EndTabItem();
            }
            
            // Technical Indicators Tab
            if (ImGui::BeginTabItem("Technical Indicators")) {
                std::vector<float> rsi_values, volatility_values, momentum_values, returns_values;
                std::vector<float> parkinson_vol_values, spread_values, internal_bar_values;
                
                for (const auto& point : data) {
                    rsi_values.push_back(point.rsi);
                    volatility_values.push_back(point.volatility);
                    momentum_values.push_back(point.momentum);
                    returns_values.push_back(point.returns);
                    parkinson_vol_values.push_back(point.parkinson_volatility_20);
                    spread_values.push_back(point.spread);
                    internal_bar_values.push_back(point.internal_bar_strength);
                }
                
                // RSI with levels
                if (ImPlot::BeginPlot(("RSI - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "RSI");
                    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
                    ImPlot::PlotLine("RSI", indices.data(), rsi_values.data(), indices.size());
                    
                    // Add overbought/oversold lines
                    std::vector<float> overbought(indices.size(), 70.0f);
                    std::vector<float> oversold(indices.size(), 30.0f);
                    ImPlot::PlotLine("Overbought", indices.data(), overbought.data(), indices.size());
                    ImPlot::PlotLine("Oversold", indices.data(), oversold.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Volatility Comparison
                if (ImPlot::BeginPlot(("Volatility Analysis - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Volatility");
                    ImPlot::PlotLine("Standard Volatility", indices.data(), volatility_values.data(), indices.size());
                    ImPlot::PlotLine("Parkinson Volatility", indices.data(), parkinson_vol_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Returns and Momentum
                if (ImPlot::BeginPlot(("Returns & Momentum - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Value");
                    ImPlot::PlotLine("Returns", indices.data(), returns_values.data(), indices.size());
                    ImPlot::PlotLine("Momentum", indices.data(), momentum_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Spread and Internal Bar Strength
                if (ImPlot::BeginPlot(("Market Microstructure - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Value");
                    ImPlot::PlotLine("Spread", indices.data(), spread_values.data(), indices.size());
                    ImPlot::PlotLine("Internal Bar Strength", indices.data(), internal_bar_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                ImGui::EndTabItem();
            }
            
            // Advanced Features Tab
            if (ImGui::BeginTabItem("Advanced Features")) {
                std::vector<float> kama_values, slope20_values, slope60_values, velocity_values;
                std::vector<float> acceleration_values, log_pct_change_values, auto_corr_values;
                
                for (const auto& point : data) {
                    kama_values.push_back(point.kama_10_2_30);
                    slope20_values.push_back(point.linear_slope_20);
                    slope60_values.push_back(point.linear_slope_60);
                    velocity_values.push_back(point.velocity);
                    acceleration_values.push_back(point.acceleration);
                    log_pct_change_values.push_back(point.log_pct_change_5);
                    auto_corr_values.push_back(point.auto_correlation_50_10);
                }
                
                // KAMA Adaptive Moving Average
                if (ImPlot::BeginPlot(("KAMA - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "KAMA");
                    ImPlot::PlotLine("KAMA", indices.data(), kama_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Linear Slopes Trend Analysis
                if (ImPlot::BeginPlot(("Trend Slopes - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Slope");
                    ImPlot::PlotLine("Linear Slope 20", indices.data(), slope20_values.data(), indices.size());
                    ImPlot::PlotLine("Linear Slope 60", indices.data(), slope60_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Velocity and Acceleration
                if (ImPlot::BeginPlot(("Motion Analysis - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Value");
                    ImPlot::PlotLine("Velocity", indices.data(), velocity_values.data(), indices.size());
                    ImPlot::PlotLine("Acceleration", indices.data(), acceleration_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Log Returns and Auto Correlation
                if (ImPlot::BeginPlot(("Statistical Measures - " + symbol).c_str(), ImVec2(-1, 150))) {
                    ImPlot::SetupAxes("Time Index", "Value");
                    ImPlot::PlotLine("Log Pct Change 5", indices.data(), log_pct_change_values.data(), indices.size());
                    ImPlot::PlotLine("Auto Correlation", indices.data(), auto_corr_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                ImGui::EndTabItem();
            }
            
            // Distribution & Shape Tab
            if (ImGui::BeginTabItem("Distribution & Shape")) {
                std::vector<float> skewness_values, kurtosis_values;
                std::vector<float> candle_way_values, candle_filling_values, candle_amplitude_values;
                
                for (const auto& point : data) {
                    skewness_values.push_back(point.skewness_30);
                    kurtosis_values.push_back(point.kurtosis_30);
                    candle_way_values.push_back(point.candle_way);
                    candle_filling_values.push_back(point.candle_filling);
                    candle_amplitude_values.push_back(point.candle_amplitude);
                }
                
                // Distribution Shape Metrics
                if (ImPlot::BeginPlot(("Distribution Metrics - " + symbol).c_str(), ImVec2(-1, 200))) {
                    ImPlot::SetupAxes("Time Index", "Value");
                    ImPlot::PlotLine("Skewness 30", indices.data(), skewness_values.data(), indices.size());
                    ImPlot::PlotLine("Kurtosis 30", indices.data(), kurtosis_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                // Candlestick Pattern Analysis
                if (ImPlot::BeginPlot(("Candle Way - " + symbol).c_str(), ImVec2(-1, 120))) {
                    ImPlot::SetupAxes("Time Index", "Candle Way");
                    ImPlot::PlotLine("Candle Way", indices.data(), candle_way_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                if (ImPlot::BeginPlot(("Candle Filling - " + symbol).c_str(), ImVec2(-1, 120))) {
                    ImPlot::SetupAxes("Time Index", "Candle Filling");
                    ImPlot::PlotLine("Candle Filling", indices.data(), candle_filling_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                if (ImPlot::BeginPlot(("Candle Amplitude - " + symbol).c_str(), ImVec2(-1, 120))) {
                    ImPlot::SetupAxes("Time Index", "Candle Amplitude");
                    ImPlot::PlotLine("Candle Amplitude", indices.data(), candle_amplitude_values.data(), indices.size());
                    ImPlot::EndPlot();
                }
                
                ImGui::EndTabItem();
            }
            
            // Summary Statistics Tab
            if (ImGui::BeginTabItem("Statistics")) {
                if (!data.empty()) {
                    const auto& latest = data.back();
                    
                    ImGui::Columns(3, "StatsColumns");
                    
                    // Column 1: Price Data
                    ImGui::Text("PRICE DATA");
                    ImGui::Separator();
                    ImGui::Text("Open: $%.2f", latest.open);
                    ImGui::Text("High: $%.2f", latest.high);
                    ImGui::Text("Low: $%.2f", latest.low);
                    ImGui::Text("Close: $%.2f", latest.close);
                    ImGui::Text("Volume: %.0f", latest.volume);
                    ImGui::Text("SMA: $%.2f", latest.sma);
                    ImGui::Text("Volume SMA: %.0f", latest.volume_sma_20);
                    
                    ImGui::NextColumn();
                    
                    // Column 2: Technical Indicators
                    ImGui::Text("TECHNICAL INDICATORS");
                    ImGui::Separator();
                    ImGui::Text("Returns: %.4f", latest.returns);
                    ImGui::Text("RSI: %.2f", latest.rsi);
                    ImGui::Text("Volatility: %.4f", latest.volatility);
                    ImGui::Text("Momentum: %.4f", latest.momentum);
                    ImGui::Text("Parkinson Vol: %.4f", latest.parkinson_volatility_20);
                    ImGui::Text("Spread: %.4f", latest.spread);
                    ImGui::Text("Internal Bar: %.4f", latest.internal_bar_strength);
                    
                    ImGui::NextColumn();
                    
                    // Column 3: Advanced Features
                    ImGui::Text("ADVANCED FEATURES");
                    ImGui::Separator();
                    ImGui::Text("KAMA: %.4f", latest.kama_10_2_30);
                    ImGui::Text("Slope 20: %.6f", latest.linear_slope_20);
                    ImGui::Text("Slope 60: %.6f", latest.linear_slope_60);
                    ImGui::Text("Velocity: %.4f", latest.velocity);
                    ImGui::Text("Acceleration: %.4f", latest.acceleration);
                    ImGui::Text("Log Pct Chg: %.6f", latest.log_pct_change_5);
                    ImGui::Text("Auto Corr: %.6f", latest.auto_correlation_50_10);
                    
                    ImGui::Columns(1);
                    ImGui::Separator();
                    
                    // Additional metrics
                    ImGui::Text("DISTRIBUTION & PATTERNS");
                    ImGui::Text("Skewness (30): %.4f", latest.skewness_30);
                    ImGui::Text("Kurtosis (30): %.4f", latest.kurtosis_30);
                    ImGui::Text("Candle Way: %.4f", latest.candle_way);
                    ImGui::Text("Candle Filling: %.4f", latest.candle_filling);
                    ImGui::Text("Candle Amplitude: %.4f", latest.candle_amplitude);
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
};

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    GLFWwindow* window = glfwCreateWindow(1400, 900, "Stock Data Visualizer - 10K Stocks", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (gl3wInit() != 0) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    StockVisualizer visualizer;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        visualizer.RenderUI();
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}