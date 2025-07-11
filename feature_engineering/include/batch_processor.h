#pragma once
#include <vector>
#include <memory>

class BatchOHLCProcessor {
private:
    mutable std::vector<double> workspace_;
    
public:
    BatchOHLCProcessor();
    ~BatchOHLCProcessor() = default;
    
    // Optimized feature set with aligned memory
    struct FeatureSet {
        alignas(32) std::vector<double> returns;
        alignas(32) std::vector<double> sma;
        alignas(32) std::vector<double> rsi;
        alignas(32) std::vector<double> volatility;
        alignas(32) std::vector<double> momentum;
        alignas(32) std::vector<double> spread;
        alignas(32) std::vector<double> internal_bar_strength;
        alignas(32) std::vector<double> skewness_30;
        alignas(32) std::vector<double> kurtosis_30;
        alignas(32) std::vector<double> log_pct_change_5;
        alignas(32) std::vector<double> auto_correlation_50_10;
        alignas(32) std::vector<double> kama_10_2_30;
        alignas(32) std::vector<double> linear_slope_20;
        alignas(32) std::vector<double> linear_slope_60;
        alignas(32) std::vector<double> parkinson_volatility_20;
        alignas(32) std::vector<double> volume_sma_20;
        alignas(32) std::vector<double> velocity;
        alignas(32) std::vector<double> acceleration;
        alignas(32) std::vector<int> candle_way;
        alignas(32) std::vector<double> candle_filling;
        alignas(32) std::vector<double> candle_amplitude;
        
        void resize_all(size_t size);
        void clear_all();
    };
    
    // High-performance organized feature calculation
    FeatureSet calculate_organized_features(
        const std::vector<double>& open,
        const std::vector<double>& high,
        const std::vector<double>& low,
        const std::vector<double>& close,
        const std::vector<double>& volume
    );
    
    // Batch processing methods
    std::vector<std::vector<double>> batch_calculate_features(
        const std::vector<std::vector<double>>& close_prices,
        const std::vector<std::vector<double>>& volumes,
        int sma_period = 20,
        int rsi_period = 14
    );
    
    std::vector<double> calculate_stock_features(
        const std::vector<double>& close_prices,
        const std::vector<double>& volumes,
        int sma_period,
        int rsi_period
    );
    
    std::vector<double> calculate_comprehensive_features(
        const std::vector<double>& open,
        const std::vector<double>& high,
        const std::vector<double>& low,
        const std::vector<double>& close,
        const std::vector<double>& volume
    );
    
    std::vector<std::vector<double>> batch_calculate_comprehensive_features(
        const std::vector<std::vector<double>>& open_prices,
        const std::vector<std::vector<double>>& high_prices,
        const std::vector<std::vector<double>>& low_prices,
        const std::vector<std::vector<double>>& close_prices,
        const std::vector<std::vector<double>>& volumes
    );
};