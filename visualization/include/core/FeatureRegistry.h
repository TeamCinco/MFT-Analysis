#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace Visualization {

enum class FeatureCategory {
    PRICE,
    TECHNICAL,
    STATISTICAL,
    VOLUME,
    REGIME,
    RISK,
    INTERACTION,
    CANDLESTICK,
    MOMENTUM,
    VOLATILITY
};

enum class ChartType {
    LINE,
    BAR,
    HISTOGRAM,
    SCATTER,
    HEATMAP,
    CANDLESTICK,
    AREA
};

struct FeatureMetadata {
    std::string name;
    std::string display_name;
    FeatureCategory category;
    ChartType preferred_chart_type;
    std::string units;
    float color[3]; // RGB values 0-1
    bool is_percentage;
    bool has_bounds;
    float min_bound;
    float max_bound;
    std::string description;
    
    FeatureMetadata(const std::string& name, 
                   const std::string& display_name,
                   FeatureCategory category,
                   ChartType chart_type = ChartType::LINE,
                   const std::string& units = "",
                   bool is_percentage = false)
        : name(name), display_name(display_name), category(category),
          preferred_chart_type(chart_type), units(units), 
          is_percentage(is_percentage), has_bounds(false),
          min_bound(0.0f), max_bound(0.0f) {
        // Default colors based on category
        setDefaultColor();
    }
    
    void setDefaultColor();
    void setBounds(float min_val, float max_val) {
        has_bounds = true;
        min_bound = min_val;
        max_bound = max_val;
    }
};

class FeatureRegistry {
public:
    static FeatureRegistry& getInstance();
    
    // Registration methods
    void registerFeature(const FeatureMetadata& metadata);
    void registerFeature(const std::string& name, 
                        const std::string& display_name,
                        FeatureCategory category,
                        ChartType chart_type = ChartType::LINE,
                        const std::string& units = "",
                        bool is_percentage = false);
    
    // Lookup methods
    const FeatureMetadata* getFeature(const std::string& name) const;
    std::vector<std::string> getFeaturesByCategory(FeatureCategory category) const;
    std::vector<std::string> getAllFeatureNames() const;
    std::vector<FeatureCategory> getAllCategories() const;
    
    // Utility methods
    std::string getCategoryName(FeatureCategory category) const;
    bool isFeatureRegistered(const std::string& name) const;
    size_t getFeatureCount() const;
    
    // Initialize with default features from FeatureSet
    void initializeDefaultFeatures();
    
private:
    FeatureRegistry() = default;
    std::unordered_map<std::string, FeatureMetadata> features_;
    std::unordered_map<FeatureCategory, std::vector<std::string>> category_map_;
    
    void updateCategoryMap(const std::string& name, FeatureCategory category);
};

// Macro for easy feature registration
#define REGISTER_FEATURE(name, category, display_name, ...) \
    do { \
        FeatureRegistry::getInstance().registerFeature(#name, display_name, \
            FeatureCategory::category, ##__VA_ARGS__); \
    } while(0)

// Helper function to register all features from FeatureSet
void registerAllFeatures();

} // namespace Visualization
