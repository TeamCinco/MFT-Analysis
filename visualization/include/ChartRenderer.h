#pragma once
#include <vector>
#include <string>
#include "StockData.h"

class ChartRenderer {
public:
    static void renderPriceVolumeCharts(const std::string& symbol, const std::vector<StockData>& data);
    static void renderTechnicalIndicators(const std::string& symbol, const std::vector<StockData>& data);
    static void renderAdvancedFeatures(const std::string& symbol, const std::vector<StockData>& data);
    static void renderDistributionShapeCharts(const std::string& symbol, const std::vector<StockData>& data);
    
    // New advanced visualization functions
    static void renderOscillators(const std::string& symbol, const std::vector<StockData>& data);
    static void renderIchimokuCloud(const std::string& symbol, const std::vector<StockData>& data);
    static void renderVolumeProfile(const std::string& symbol, const std::vector<StockData>& data);
    static void renderStatisticalMeasures(const std::string& symbol, const std::vector<StockData>& data);
    static void renderRiskMetrics(const std::string& symbol, const std::vector<StockData>& data);
    static void renderRegimeAnalysis(const std::string& symbol, const std::vector<StockData>& data);
    
    static void renderStatistics(const std::vector<StockData>& data);
    
private:
    static std::vector<float> createIndices(size_t size);
    static void extractPriceVolumeData(const std::vector<StockData>& data,
                                      std::vector<float>& opens, std::vector<float>& highs,
                                      std::vector<float>& lows, std::vector<float>& closes,
                                      std::vector<float>& volumes, std::vector<float>& sma_values,
                                      std::vector<float>& volume_sma_values);
};
