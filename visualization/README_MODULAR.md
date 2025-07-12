# Modular Visualization System

A super modular visualization system that automatically adapts to new features in your `FeatureSet`. Adding new features is now as simple as updating the `FeatureSet` struct - the visualization system automatically discovers and visualizes them!

## 🚀 Key Benefits

- **Zero-code feature addition**: Add features to `FeatureSet` and they automatically appear in visualizations
- **Automatic chart generation**: Charts are created dynamically based on feature metadata
- **Configurable layouts**: Grid, tabs, accordion, and custom layouts
- **Type-safe extraction**: Template-based feature extraction with compile-time safety
- **Plugin architecture**: Easy to extend with new chart types and renderers
- **Category-based organization**: Features are automatically grouped by type

## 📁 Architecture Overview

```
visualization/
├── include/
│   ├── core/                          # Core modular infrastructure
│   │   ├── FeatureRegistry.h          # Dynamic feature registration & metadata
│   │   ├── FeatureExtractor.h         # Bridge between FeatureSet and visualization
│   │   └── ChartFactory.h             # Dynamic chart generation
│   ├── rendering/
│   │   └── ModularChartRenderer.h     # New modular renderer
│   └── VisualizationManager.h         # High-level coordinator
├── src/
│   ├── core/
│   │   ├── FeatureRegistry.cpp
│   │   ├── FeatureExtractor.cpp
│   │   └── ChartFactory.cpp
│   ├── rendering/
│   │   └── ModularChartRenderer.cpp
│   └── VisualizationManager.cpp
└── examples/
    └── modular_example.cpp            # Usage examples
```

## 🔧 How It Works

### 1. Feature Registry
Automatically registers all features from your `FeatureSet` with metadata:
- Display name and units
- Category (Technical, Statistical, Volume, etc.)
- Preferred chart type (Line, Bar, Histogram, etc.)
- Color scheme and bounds

### 2. Feature Extractor
Bridges between your `FeatureSet` and the visualization system:
- Template-based extraction for type safety
- Automatic feature discovery using reflection-like techniques
- Handles missing/invalid data gracefully

### 3. Chart Factory
Dynamically creates charts based on feature metadata:
- Single feature charts
- Category-based multi-feature charts
- Comparison charts
- Statistical distribution charts

### 4. Modular Renderer
Replaces the hard-coded renderer with a flexible system:
- Multiple layout types (Grid, Tabs, Accordion)
- Configurable filters and selections
- Interactive controls and statistics

## 🎯 Usage Examples

### Basic Usage

```cpp
#include "VisualizationManager.h"

// Initialize the system
VisualizationManager viz_manager;
viz_manager.initialize();

// Load data from FeatureSet
viz_manager.loadData(symbol, ohlcv_data, feature_set);

// Render in your main loop
while (running) {
    viz_manager.renderMainDashboard();
    // Handle ImGui rendering...
}
```

### Load from CSV
```cpp
viz_manager.loadData("path/to/AAPL_features.csv");
```

### Configure Layout
```cpp
DashboardLayout layout(DashboardLayout::LayoutType::TABS, 3, 2);
viz_manager.setDashboardLayout(layout);

ChartFilter filter;
filter.show_technical_indicators = true;
filter.show_statistical_features = false;
viz_manager.setChartFilter(filter);
```

### Add Custom Features
```cpp
viz_manager.addFeature("my_indicator", "My Custom Indicator", 
                      FeatureCategory::TECHNICAL, ChartType::LINE);
```

### EasyViz Convenience Functions
```cpp
// One-liner visualizations
EasyViz::visualizeCSV("data.csv");
EasyViz::visualizeFeatureSet(symbol, ohlcv_data, feature_set);
EasyViz::compareFeatures({"rsi", "momentum"}, "data.csv");
EasyViz::visualizeCategory(FeatureCategory::TECHNICAL, "data.csv");
```

## ➕ Adding New Features

Adding a new feature is incredibly simple:

### Step 1: Add to FeatureSet
```cpp
struct FeatureSet {
    // ... existing features ...
    std::vector<double> my_new_feature;  // Add your feature here
};
```

### Step 2: Add Extraction Logic
```cpp
// In FeatureExtractor::initializeExtractors()
feature_extractors_["my_new_feature"] = [](const FeatureSet& fs) { 
    return extractVectorFromMember(fs.my_new_feature); 
};
```

### Step 3: Register the Feature
```cpp
// In FeatureRegistry::initializeDefaultFeatures()
registerFeature("my_new_feature", "My New Feature", 
               FeatureCategory::TECHNICAL, ChartType::LINE);
```

**That's it!** Your feature now automatically appears in:
- Category dashboards
- Feature selectors
- Statistical summaries
- Comparison charts
- All layout types

## 🎨 Feature Categories

Features are automatically organized into categories:

- **PRICE**: OHLC price data
- **TECHNICAL**: Technical indicators (RSI, SMA, etc.)
- **STATISTICAL**: Statistical measures (skewness, kurtosis, etc.)
- **VOLUME**: Volume-based indicators
- **VOLATILITY**: Volatility measures
- **MOMENTUM**: Momentum indicators
- **REGIME**: Regime detection features
- **RISK**: Risk measures (VaR, drawdown, etc.)
- **INTERACTION**: Feature interactions
- **CANDLESTICK**: Candlestick patterns

## 📊 Chart Types

The system supports multiple chart types:

- **LINE**: Standard line charts
- **BAR**: Bar charts (great for volume)
- **AREA**: Filled area charts
- **SCATTER**: Scatter plots
- **HISTOGRAM**: Distribution histograms
- **HEATMAP**: Correlation matrices

## 🔧 Configuration Options

### Dashboard Layouts
- **Grid**: Configurable rows/columns
- **Tabs**: Organized by categories
- **Accordion**: Collapsible sections
- **Custom**: Define your own layout

### Chart Filters
- Show/hide specific categories
- Enable/disable individual features
- Filter by feature types

### UI Controls
- Interactive feature selection
- Real-time layout switching
- Statistical summaries
- Data export capabilities

## 🚀 Performance Features

- **Chart Caching**: Avoid regenerating identical charts
- **Lazy Loading**: Charts generated only when needed
- **Memory Efficient**: Smart data structures minimize memory usage
- **Fast Rendering**: Optimized ImPlot integration

## 🔄 Migration from Old System

The new system is designed to coexist with the existing `ChartRenderer`. You can:

1. **Gradual Migration**: Use both systems side-by-side
2. **Feature-by-Feature**: Migrate individual charts over time
3. **Complete Switch**: Replace `ChartRenderer` with `ModularChartRenderer`

### Migration Example
```cpp
// Old way
ChartRenderer::renderTechnicalIndicators(symbol, data);

// New way
ModularChartRenderer renderer;
renderer.renderCategoryChart(FeatureCategory::TECHNICAL, data, symbol);
```

## 🎯 Real-World Example

```cpp
// Your feature engineering produces a FeatureSet with 50+ features
FeatureSet features = calculateAllFeatures(ohlcv_data);

// Load into visualization system
VisualizationManager viz;
viz.initialize();
viz.loadData("AAPL", ohlcv_data, features);

// Configure for your needs
DashboardLayout layout(DashboardLayout::LayoutType::TABS);
viz.setDashboardLayout(layout);

// Render everything automatically
while (app_running) {
    viz.renderMainDashboard();  // All 50+ features automatically visualized!
    
    // ImGui/ImPlot rendering...
}
```

## 🔮 Future Extensibility

The modular design makes it easy to add:

- **New Chart Types**: Candlestick, 3D plots, etc.
- **Export Formats**: PNG, SVG, PDF export
- **Interactive Features**: Zoom, pan, selection
- **Real-time Updates**: Live data streaming
- **Custom Layouts**: Domain-specific arrangements
- **Plugin System**: Third-party extensions

## 🏗️ Building

The modular system integrates with your existing CMake setup. All new files are automatically included in the build process.

```bash
cd visualization
mkdir build && cd build
cmake ..
make
```

## 📝 Notes

- The system automatically handles missing features gracefully
- All charts respect the feature metadata (colors, bounds, units)
- Statistical calculations are performed on-demand
- The UI is fully interactive with ImGui controls
- Memory usage scales efficiently with data size

---

**The modular visualization system transforms feature addition from a multi-file editing task into a simple struct update!**
