#pragma once

#include "ohlcv_data.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>

class BatchOHLCProcessor {
private:
    class ComputationCache {
    private:
        std::map<std::string, std::vector<double>> cache_;
        bool use_simd_;

    public:
        ComputationCache(bool enable_simd = true);
        std::vector<double>& get_or_compute(
            const std::string& key,
            std::function<std::vector<double>()> simd_computer,
            std::function<std::vector<double>()> scalar_computer
        );
        void clear();
    };

public:
    BatchOHLCProcessor() = default;

    FeatureSet calculate_features(
        const std::vector<double>& open,
        const std::vector<double>& high,
        const std::vector<double>& low,
        const std::vector<double>& close,
        const std::vector<double>& volume,
        bool force_scalar = false
    );

    std::vector<FeatureSet> batch_calculate_features(
        const std::vector<std::vector<double>>& open_prices,
        const std::vector<std::vector<double>>& high_prices,
        const std::vector<std::vector<double>>& low_prices,
        const std::vector<std::vector<double>>& close_prices,
        const std::vector<std::vector<double>>& volumes,
        bool force_scalar = false
    );
};
