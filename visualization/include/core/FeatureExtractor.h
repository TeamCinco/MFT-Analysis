#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <chrono>

// Forward declarations
struct FeatureSet;
struct OHLCVData;

namespace Visualization {

// Flexible data container for visualization
struct FlexibleStockData {
    std::string symbol;
    std::chrono::system_clock::time_point timestamp;
    std::string date_string;
    float datetime_index;
    
    // Dynamic feature storage
    std::unordered_map<std::string, double> features;
    
    // Convenience accessors for common features
    double getFeature(const std::string& name, double default_value = 0.0) const;
    void setFeature(const std::string& name, double value);
    bool hasFeature(const std::string& name) const;
    
    // Get all feature names
    std::vector<std::string> getFeatureNames() const;
};

// Type trait to check if a type has a specific member
template<typename T, typename = void>
struct has_member : std::false_type {};

template<typename T>
struct has_member<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

class FeatureExtractor {
public:
    // Extract features from FeatureSet to FlexibleStockData
    static std::vector<FlexibleStockData> extractFromFeatureSet(
        const std::string& symbol,
        const OHLCVData& ohlcv_data,
        const FeatureSet& feature_set);
    
    // Extract specific feature vector by name
    static std::vector<double> extractFeatureVector(
        const FeatureSet& feature_set, 
        const std::string& feature_name);
    
    // Extract multiple features at once
    static std::unordered_map<std::string, std::vector<double>> extractMultipleFeatures(
        const FeatureSet& feature_set,
        const std::vector<std::string>& feature_names);
    
    // Get all available feature names from a FeatureSet
    static std::vector<std::string> getAvailableFeatures(const FeatureSet& feature_set);
    
    // Validate that feature exists and has data
    static bool validateFeature(const FeatureSet& feature_set, const std::string& feature_name);
    
    // Convert timestamps to datetime indices for plotting
    static std::vector<float> convertTimestampsToIndices(
        const std::vector<std::chrono::system_clock::time_point>& timestamps);
    
    // Convert timestamps to date strings
    static std::vector<std::string> convertTimestampsToStrings(
        const std::vector<std::chrono::system_clock::time_point>& timestamps);

private:
    // Template function to extract vector from FeatureSet member
    template<typename T>
    static std::vector<double> extractVectorFromMember(const std::vector<T>& member_vector);
    
    // Reflection-like feature extraction using function pointers
    using FeatureExtractorFunc = std::function<std::vector<double>(const FeatureSet&)>;
    static std::unordered_map<std::string, FeatureExtractorFunc> createFeatureExtractorMap();
    static std::unordered_map<std::string, FeatureExtractorFunc> feature_extractors_;
    static bool extractors_initialized_;
    
    // Initialize the feature extractor map
    static void initializeExtractors();
    
    // Helper to ensure extractors are initialized
    static void ensureExtractorsInitialized();
};

// Template implementation
template<typename T>
std::vector<double> FeatureExtractor::extractVectorFromMember(const std::vector<T>& member_vector) {
    std::vector<double> result;
    result.reserve(member_vector.size());
    
    for (const auto& value : member_vector) {
        if constexpr (std::is_same_v<T, double>) {
            result.push_back(value);
        } else if constexpr (std::is_same_v<T, float>) {
            result.push_back(static_cast<double>(value));
        } else if constexpr (std::is_same_v<T, int>) {
            result.push_back(static_cast<double>(value));
        } else {
            // For other types, attempt static_cast to double
            result.push_back(static_cast<double>(value));
        }
    }
    
    return result;
}

} // namespace Visualization
