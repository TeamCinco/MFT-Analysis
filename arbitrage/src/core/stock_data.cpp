#include "stock_data.h"
#include <algorithm>
#include <numeric>
#include <cmath>

void StockData::calculateStatistics() {
    if (close.empty()) {
        return;
    }
    
    // Calculate basic price statistics
    min_price = *std::min_element(close.begin(), close.end());
    max_price = *std::max_element(close.begin(), close.end());
    mean_price = std::accumulate(close.begin(), close.end(), 0.0) / close.size();
    
    // Calculate volatility (standard deviation of returns)
    if (!returns.empty()) {
        mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        
        double variance = 0.0;
        for (double ret : returns) {
            variance += (ret - mean_return) * (ret - mean_return);
        }
        variance /= returns.size();
        volatility = std::sqrt(variance);
    }
}

void StockData::calculateReturns() {
    if (close.size() < 2) {
        return;
    }
    
    returns.clear();
    returns.reserve(close.size() - 1);
    
    for (size_t i = 1; i < close.size(); ++i) {
        if (close[i-1] != 0.0) {
            double ret = (close[i] - close[i-1]) / close[i-1];
            returns.push_back(ret);
        } else {
            returns.push_back(0.0);
        }
    }
}
