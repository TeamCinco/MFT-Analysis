#include "ChartRenderer.h"
#include "imgui.h"
#include "implot.h"

std::vector<float> ChartRenderer::createIndices(size_t size) {
    std::vector<float> indices;
    for (size_t i = 0; i < size; ++i) {
        indices.push_back((float)i);
    }
    return indices;
}

void ChartRenderer::extractPriceVolumeData(const std::vector<StockData>& data,
                                          std::vector<float>& opens, std::vector<float>& highs,
                                          std::vector<float>& lows, std::vector<float>& closes,
                                          std::vector<float>& volumes, std::vector<float>& sma_values,
                                          std::vector<float>& volume_sma_values) {
    for (const auto& point : data) {
        opens.push_back(point.open);
        highs.push_back(point.high);
        lows.push_back(point.low);
        closes.push_back(point.close);
        volumes.push_back(point.volume);
        sma_values.push_back(point.sma);
        volume_sma_values.push_back(point.volume_sma_20);
    }
}

void ChartRenderer::renderPriceVolumeCharts(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> opens, highs, lows, closes, volumes, sma_values, volume_sma_values;
    
    extractPriceVolumeData(data, opens, highs, lows, closes, volumes, sma_values, volume_sma_values);
    
    // OHLC Price Chart
    if (ImPlot::BeginPlot(("OHLC Price - " + symbol).c_str(), ImVec2(-1, 300))) {
        ImPlot::SetupAxes("Time Index", "Price ($)");
        ImPlot::PlotLine("Open", indices.data(), opens.data(), indices.size());
        ImPlot::PlotLine("High", indices.data(), highs.data(), indices.size());
        ImPlot::PlotLine("Low", indices.data(), lows.data(), indices.size());
        ImPlot::PlotLine("Close", indices.data(), closes.data(), indices.size());
        ImPlot::PlotLine("SMA", indices.data(), sma_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Volume Chart with SMA
    if (ImPlot::BeginPlot(("Volume Analysis - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Volume");
        ImPlot::PlotBars("Volume", indices.data(), volumes.data(), indices.size(), 0.8);
        ImPlot::PlotLine("Volume SMA 20", indices.data(), volume_sma_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderTechnicalIndicators(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> rsi_values, volatility_values, momentum_values, returns_values;
    std::vector<float> parkinson_vol_values, spread_values, internal_bar_values;
    
    for (const auto& point : data) {
        rsi_values.push_back(point.rsi);
        volatility_values.push_back(point.volatility);
        momentum_values.push_back(point.momentum);
        returns_values.push_back(point.returns);
        parkinson_vol_values.push_back(point.parkinson_volatility_20);
        spread_values.push_back(point.spread);
        internal_bar_values.push_back(point.internal_bar_strength);
    }
    
    // RSI with levels
    if (ImPlot::BeginPlot(("RSI - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "RSI");
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
        ImPlot::PlotLine("RSI", indices.data(), rsi_values.data(), indices.size());
        
        // Add overbought/oversold lines
        std::vector<float> overbought(indices.size(), 70.0f);
        std::vector<float> oversold(indices.size(), 30.0f);
        ImPlot::PlotLine("Overbought", indices.data(), overbought.data(), indices.size());
        ImPlot::PlotLine("Oversold", indices.data(), oversold.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Volatility Comparison
    if (ImPlot::BeginPlot(("Volatility Analysis - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Volatility");
        ImPlot::PlotLine("Standard Volatility", indices.data(), volatility_values.data(), indices.size());
        ImPlot::PlotLine("Parkinson Volatility", indices.data(), parkinson_vol_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Returns and Momentum
    if (ImPlot::BeginPlot(("Returns & Momentum - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Returns", indices.data(), returns_values.data(), indices.size());
        ImPlot::PlotLine("Momentum", indices.data(), momentum_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Spread and Internal Bar Strength
    if (ImPlot::BeginPlot(("Market Microstructure - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Spread", indices.data(), spread_values.data(), indices.size());
        ImPlot::PlotLine("Internal Bar Strength", indices.data(), internal_bar_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderAdvancedFeatures(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> kama_values, slope20_values, slope60_values, velocity_values;
    std::vector<float> acceleration_values, log_pct_change_values, auto_corr_values;
    
    for (const auto& point : data) {
        kama_values.push_back(point.kama_10_2_30);
        slope20_values.push_back(point.linear_slope_20);
        slope60_values.push_back(point.linear_slope_60);
        velocity_values.push_back(point.velocity);
        acceleration_values.push_back(point.acceleration);
        log_pct_change_values.push_back(point.log_pct_change_5);
        auto_corr_values.push_back(point.auto_correlation_50_10);
    }
    
    // KAMA Adaptive Moving Average
    if (ImPlot::BeginPlot(("KAMA - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "KAMA");
        ImPlot::PlotLine("KAMA", indices.data(), kama_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Linear Slopes Trend Analysis
    if (ImPlot::BeginPlot(("Trend Slopes - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Slope");
        ImPlot::PlotLine("Linear Slope 20", indices.data(), slope20_values.data(), indices.size());
        ImPlot::PlotLine("Linear Slope 60", indices.data(), slope60_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Velocity and Acceleration
    if (ImPlot::BeginPlot(("Motion Analysis - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Velocity", indices.data(), velocity_values.data(), indices.size());
        ImPlot::PlotLine("Acceleration", indices.data(), acceleration_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Log Returns and Auto Correlation
    if (ImPlot::BeginPlot(("Statistical Measures - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Log Pct Change 5", indices.data(), log_pct_change_values.data(), indices.size());
        ImPlot::PlotLine("Auto Correlation", indices.data(), auto_corr_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderDistributionShapeCharts(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> skewness_values, kurtosis_values;
    std::vector<float> candle_way_values, candle_filling_values, candle_amplitude_values;
    
    for (const auto& point : data) {
        skewness_values.push_back(point.skewness_30);
        kurtosis_values.push_back(point.kurtosis_30);
        candle_way_values.push_back(point.candle_way);
        candle_filling_values.push_back(point.candle_filling);
        candle_amplitude_values.push_back(point.candle_amplitude);
    }
    
    // Distribution Shape Metrics
    if (ImPlot::BeginPlot(("Distribution Metrics - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Skewness 30", indices.data(), skewness_values.data(), indices.size());
        ImPlot::PlotLine("Kurtosis 30", indices.data(), kurtosis_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Candlestick Pattern Analysis
    if (ImPlot::BeginPlot(("Candle Way - " + symbol).c_str(), ImVec2(-1, 120))) {
        ImPlot::SetupAxes("Time Index", "Candle Way");
        ImPlot::PlotLine("Candle Way", indices.data(), candle_way_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    if (ImPlot::BeginPlot(("Candle Filling - " + symbol).c_str(), ImVec2(-1, 120))) {
        ImPlot::SetupAxes("Time Index", "Candle Filling");
        ImPlot::PlotLine("Candle Filling", indices.data(), candle_filling_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    if (ImPlot::BeginPlot(("Candle Amplitude - " + symbol).c_str(), ImVec2(-1, 120))) {
        ImPlot::SetupAxes("Time Index", "Candle Amplitude");
        ImPlot::PlotLine("Candle Amplitude", indices.data(), candle_amplitude_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderStatistics(const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    const auto& latest = data.back();
    
    ImGui::Columns(3, "StatsColumns");
    
    // Column 1: Price Data
    ImGui::Text("PRICE DATA");
    ImGui::Separator();
    ImGui::Text("Open: $%.2f", latest.open);
    ImGui::Text("High: $%.2f", latest.high);
    ImGui::Text("Low: $%.2f", latest.low);
    ImGui::Text("Close: $%.2f", latest.close);
    ImGui::Text("Volume: %.0f", latest.volume);
    ImGui::Text("SMA: $%.2f", latest.sma);
    ImGui::Text("Volume SMA: %.0f", latest.volume_sma_20);
    
    ImGui::NextColumn();
    
    // Column 2: Technical Indicators
    ImGui::Text("TECHNICAL INDICATORS");
    ImGui::Separator();
    ImGui::Text("Returns: %.4f", latest.returns);
    ImGui::Text("RSI: %.2f", latest.rsi);
    ImGui::Text("Volatility: %.4f", latest.volatility);
    ImGui::Text("Momentum: %.4f", latest.momentum);
    ImGui::Text("Parkinson Vol: %.4f", latest.parkinson_volatility_20);
    ImGui::Text("Spread: %.4f", latest.spread);
    ImGui::Text("Internal Bar: %.4f", latest.internal_bar_strength);
    
    ImGui::NextColumn();
    
    // Column 3: Advanced Features
    ImGui::Text("ADVANCED FEATURES");
    ImGui::Separator();
    ImGui::Text("KAMA: %.4f", latest.kama_10_2_30);
    ImGui::Text("Slope 20: %.6f", latest.linear_slope_20);
    ImGui::Text("Slope 60: %.6f", latest.linear_slope_60);
    ImGui::Text("Velocity: %.4f", latest.velocity);
    ImGui::Text("Acceleration: %.4f", latest.acceleration);
    ImGui::Text("Log Pct Chg: %.6f", latest.log_pct_change_5);
    ImGui::Text("Auto Corr: %.6f", latest.auto_correlation_50_10);
    
    ImGui::Columns(1);
    ImGui::Separator();
    
    // Additional metrics
    ImGui::Text("DISTRIBUTION & PATTERNS");
    ImGui::Text("Skewness (30): %.4f", latest.skewness_30);
    ImGui::Text("Kurtosis (30): %.4f", latest.kurtosis_30);
    ImGui::Text("Candle Way: %.4f", latest.candle_way);
    ImGui::Text("Candle Filling: %.4f", latest.candle_filling);
    ImGui::Text("Candle Amplitude: %.4f", latest.candle_amplitude);
}