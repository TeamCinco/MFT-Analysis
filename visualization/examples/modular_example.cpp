#include "../include/VisualizationManager.h"
#include "../../feature_engineering/include/ohlcv_data.h"
#include <iostream>

using namespace Visualization;

int main() {
    std::cout << "Modular Visualization System Example\n";
    std::cout << "====================================\n\n";
    
    // Initialize the visualization manager
    VisualizationManager viz_manager;
    viz_manager.initialize();
    
    std::cout << "1. Loading demo data...\n";
    viz_manager.loadDemoData();
    
    // Example 1: Load data from CSV
    std::cout << "2. Example: Loading from CSV\n";
    std::string csv_path = "../data/AAPL_features.csv";
    if (viz_manager.loadData(csv_path)) {
        std::cout << "   Successfully loaded: " << csv_path << "\n";
        std::cout << "   Available symbols: ";
        for (const auto& symbol : viz_manager.getAvailableSymbols()) {
            std::cout << symbol << " ";
        }
        std::cout << "\n";
    }
    
    // Example 2: Add a custom feature
    std::cout << "\n3. Example: Adding custom feature\n";
    viz_manager.addFeature("custom_indicator", "Custom Technical Indicator", 
                          FeatureCategory::TECHNICAL, ChartType::LINE);
    std::cout << "   Added custom feature to registry\n";
    
    // Example 3: Configure dashboard layout
    std::cout << "\n4. Example: Configuring dashboard\n";
    DashboardLayout layout(DashboardLayout::LayoutType::TABS, 3, 2);
    viz_manager.setDashboardLayout(layout);
    
    ChartFilter filter;
    filter.show_price_charts = true;
    filter.show_volume_charts = true;
    filter.show_technical_indicators = true;
    filter.show_statistical_features = false; // Hide statistical features
    viz_manager.setChartFilter(filter);
    std::cout << "   Dashboard configured with tabs layout\n";
    
    // Example 4: Using FeatureSet directly
    std::cout << "\n5. Example: Using FeatureSet directly\n";
    
    // Create sample OHLCV data
    OHLCVData ohlcv_data;
    ohlcv_data.symbol = "SAMPLE";
    
    // Add some sample data points
    for (int i = 0; i < 50; ++i) {
        ohlcv_data.timestamps.push_back(std::chrono::system_clock::now() + std::chrono::hours(i));
        ohlcv_data.open.push_back(100.0 + i * 0.5);
        ohlcv_data.high.push_back(102.0 + i * 0.5);
        ohlcv_data.low.push_back(98.0 + i * 0.5);
        ohlcv_data.close.push_back(101.0 + i * 0.5);
        ohlcv_data.volume.push_back(1000000 + i * 10000);
    }
    
    // Create sample FeatureSet
    FeatureSet feature_set;
    for (int i = 0; i < 50; ++i) {
        feature_set.returns.push_back(0.01 + (rand() % 100 - 50) / 10000.0);
        feature_set.sma.push_back(100.5 + i * 0.5);
        feature_set.rsi.push_back(30 + (rand() % 40));
        feature_set.volatility.push_back(0.15 + (rand() % 100) / 10000.0);
        feature_set.momentum.push_back((rand() % 200 - 100) / 1000.0);
        
        // Add some advanced features
        feature_set.z_score_20.push_back((rand() % 400 - 200) / 100.0);
        feature_set.kama_10_2_30.push_back(100.0 + i * 0.5 + (rand() % 100 - 50) / 100.0);
        feature_set.linear_slope_20.push_back((rand() % 200 - 100) / 10000.0);
        feature_set.parkinson_volatility_20.push_back(0.12 + (rand() % 100) / 10000.0);
        feature_set.volume_sma_20.push_back(1000000 + i * 10000);
        
        // Candlestick features
        feature_set.candle_way.push_back(rand() % 3 - 1); // -1, 0, 1
        feature_set.candle_filling.push_back((rand() % 100) / 100.0);
        feature_set.candle_amplitude.push_back((rand() % 500) / 1000.0);
    }
    
    // Load the FeatureSet data
    if (viz_manager.loadData("SAMPLE", ohlcv_data, feature_set)) {
        std::cout << "   Successfully loaded FeatureSet data for SAMPLE\n";
        std::cout << "   Data points: " << viz_manager.getDataPointCount("SAMPLE") << "\n";
        std::cout << "   Available features: " << viz_manager.getAvailableFeatures().size() << "\n";
    }
    
    // Example 5: Show how easy it is to add new features
    std::cout << "\n6. Example: Adding new features is now trivial!\n";
    std::cout << "   To add a new feature to FeatureSet:\n";
    std::cout << "   1. Add the vector to FeatureSet struct\n";
    std::cout << "   2. Add extraction logic to FeatureExtractor\n";
    std::cout << "   3. Register it in FeatureRegistry\n";
    std::cout << "   4. It automatically appears in all visualizations!\n";
    
    // Example 6: EasyViz convenience functions
    std::cout << "\n7. Example: EasyViz convenience functions\n";
    std::cout << "   EasyViz::visualizeCSV(\"path/to/file.csv\");\n";
    std::cout << "   EasyViz::visualizeFeatureSet(symbol, ohlcv_data, feature_set);\n";
    std::cout << "   EasyViz::compareFeatures({\"rsi\", \"momentum\"}, \"file.csv\");\n";
    std::cout << "   EasyViz::visualizeCategory(FeatureCategory::TECHNICAL, \"file.csv\");\n";
    
    std::cout << "\n8. Summary of Modular Benefits:\n";
    std::cout << "   ✓ Zero-code feature addition\n";
    std::cout << "   ✓ Automatic chart generation\n";
    std::cout << "   ✓ Configurable layouts and filters\n";
    std::cout << "   ✓ Type-safe feature extraction\n";
    std::cout << "   ✓ Plugin-like architecture\n";
    std::cout << "   ✓ Easy integration with existing code\n";
    
    std::cout << "\n9. Usage in your main application:\n";
    std::cout << "   // In your main loop:\n";
    std::cout << "   while (running) {\n";
    std::cout << "       viz_manager.renderMainDashboard();\n";
    std::cout << "       // Handle ImGui rendering...\n";
    std::cout << "   }\n";
    
    return 0;
}
