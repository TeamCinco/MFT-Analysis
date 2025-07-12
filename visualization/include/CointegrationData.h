#pragma once
#include <string>

struct CointegrationData {
    std::string stock1;
    std::string stock2;
    float adf_statistic;
    float p_value;
    float critical_1pct;
    float critical_5pct;
    float critical_10pct;
    float half_life_days;
    float hedge_ratio;
    float spread_mean;
    float spread_stddev;
    float max_spread;
    float min_spread;
    float current_spread;
    float z_score;
    std::string grade;
    bool is_cointegrated;
    float entry_threshold;
    float exit_threshold;
    float expected_return;
    float sharpe_ratio;
    int historical_trades;
    float win_rate;
    
    // Helper methods for analysis
    bool isStatisticallySignificant() const {
        return p_value <= 0.05;
    }
    
    bool isHighQuality() const {
        return grade == "A" && sharpe_ratio > 1.0 && win_rate > 0.6;
    }
    
    bool isOutlier() const {
        return abs(z_score) > 2.5 || sharpe_ratio > 2.0 || win_rate > 0.8;
    }
    
    std::string getPairName() const {
        return stock1 + "/" + stock2;
    }
};
