#pragma once
#include <vector>
#include <string>
#include <map>
#include "CointegrationData.h"

class CointegrationVisualizer {
private:
    std::vector<CointegrationData> pairsData;
    bool dataLoaded = false;
    bool isLoading = false;
    std::string loadingStatus = "";
    
    // UI state
    int selectedPair = 0;
    bool showOnlyCointegrated = false;
    bool showOnlyHighQuality = false;
    bool showOnlyOutliers = false;
    std::string gradeFilter = "All";
    float minSharpeRatio = 0.0f;
    float maxHalfLife = 1000.0f;
    
    // Chart data vectors for ImPlot
    std::vector<float> adf_stats;
    std::vector<float> p_values;
    std::vector<float> half_lives;
    std::vector<float> expected_returns;
    std::vector<float> sharpe_ratios;
    std::vector<float> z_scores;
    std::vector<float> win_rates;
    std::vector<std::string> pair_names;
    
    // Analysis results
    struct AnalysisResults {
        int totalPairs = 0;
        int cointegrated = 0;
        int highQuality = 0;
        int outliers = 0;
        float avgSharpeRatio = 0.0f;
        float avgWinRate = 0.0f;
        float avgHalfLife = 0.0f;
    } analysisResults;
    
public:
    void loadCSVFile(const std::string& filename);
    void renderUI();
    void renderDashboard();
    void renderScatterPlots();
    void renderDistributions();
    void renderTopOpportunities();
    void renderPairDetails();
    void renderFilters();
    
    // Analysis functions
    void updateAnalysis();
    void applyFilters();
    std::vector<CointegrationData> getFilteredData() const;
    void exportFilteredData(const std::string& filename) const;
    
    // Utility functions
    int getTotalPairs() const { return pairsData.size(); }
    bool isDataLoaded() const { return dataLoaded; }
    void clearData();
};
