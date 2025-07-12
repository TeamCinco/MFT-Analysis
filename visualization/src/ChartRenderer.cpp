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

void ChartRenderer::renderOscillators(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> chande_momentum_values, aroon_values, trix_values, vortex_values;
    std::vector<float> fisher_transform_values, money_flow_values, klinger_values;
    
    for (const auto& point : data) {
        chande_momentum_values.push_back(point.chande_momentum_oscillator_14);
        aroon_values.push_back(point.aroon_oscillator_25);
        trix_values.push_back(point.trix_15);
        vortex_values.push_back(point.vortex_indicator_14);
        fisher_transform_values.push_back(point.fisher_transform_10);
        money_flow_values.push_back(point.money_flow_index_14);
        klinger_values.push_back(point.klinger_oscillator_34_55);
    }
    
    // Momentum Oscillators
    if (ImPlot::BeginPlot(("Momentum Oscillators - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Chande Momentum", indices.data(), chande_momentum_values.data(), indices.size());
        ImPlot::PlotLine("Aroon Oscillator", indices.data(), aroon_values.data(), indices.size());
        ImPlot::PlotLine("TRIX", indices.data(), trix_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Volume-Based Oscillators
    if (ImPlot::BeginPlot(("Volume Oscillators - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Vortex Indicator", indices.data(), vortex_values.data(), indices.size());
        ImPlot::PlotLine("Money Flow Index", indices.data(), money_flow_values.data(), indices.size());
        ImPlot::PlotLine("Klinger Oscillator", indices.data(), klinger_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Fisher Transform
    if (ImPlot::BeginPlot(("Fisher Transform - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Fisher Transform");
        ImPlot::PlotLine("Fisher Transform", indices.data(), fisher_transform_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderIchimokuCloud(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> closes, senkou_a_values, senkou_b_values, supertrend_values;
    
    for (const auto& point : data) {
        closes.push_back(point.close);
        senkou_a_values.push_back(point.ichimoku_senkou_span_A_9_26);
        senkou_b_values.push_back(point.ichimoku_senkou_span_B_26_52);
        supertrend_values.push_back(point.supertrend_10_3);
    }
    
    // Ichimoku Cloud with Price
    if (ImPlot::BeginPlot(("Ichimoku Cloud - " + symbol).c_str(), ImVec2(-1, 300))) {
        ImPlot::SetupAxes("Time Index", "Price ($)");
        ImPlot::PlotLine("Close Price", indices.data(), closes.data(), indices.size());
        ImPlot::PlotLine("Senkou Span A", indices.data(), senkou_a_values.data(), indices.size());
        ImPlot::PlotLine("Senkou Span B", indices.data(), senkou_b_values.data(), indices.size());
        ImPlot::PlotLine("SuperTrend", indices.data(), supertrend_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderVolumeProfile(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> vwap_values, vwap_dev_values, obv_sma_values;
    std::vector<float> hvn_values, lvn_values, shannon_entropy_values;
    
    for (const auto& point : data) {
        vwap_values.push_back(point.volume_weighted_average_price_intraday);
        vwap_dev_values.push_back(point.vwap_deviation_stddev_30);
        obv_sma_values.push_back(point.on_balance_volume_sma_20);
        hvn_values.push_back(point.volume_profile_high_volume_node_intraday);
        lvn_values.push_back(point.volume_profile_low_volume_node_intraday);
        shannon_entropy_values.push_back(point.shannon_entropy_volume_10);
    }
    
    // VWAP Analysis
    if (ImPlot::BeginPlot(("VWAP Analysis - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Price ($)");
        ImPlot::PlotLine("VWAP", indices.data(), vwap_values.data(), indices.size());
        ImPlot::PlotLine("VWAP Deviation", indices.data(), vwap_dev_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Volume Profile Nodes
    if (ImPlot::BeginPlot(("Volume Profile - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Price ($)");
        ImPlot::PlotLine("High Volume Node", indices.data(), hvn_values.data(), indices.size());
        ImPlot::PlotLine("Low Volume Node", indices.data(), lvn_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Volume Entropy and OBV
    if (ImPlot::BeginPlot(("Volume Metrics - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Shannon Entropy", indices.data(), shannon_entropy_values.data(), indices.size());
        ImPlot::PlotLine("OBV SMA", indices.data(), obv_sma_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderStatisticalMeasures(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> z_score_values, percentile_rank_values, coeff_var_values;
    std::vector<float> dpo_values, hurst_values, garch_vol_values;
    
    for (const auto& point : data) {
        z_score_values.push_back(point.z_score_20);
        percentile_rank_values.push_back(point.percentile_rank_50);
        coeff_var_values.push_back(point.coefficient_of_variation_30);
        dpo_values.push_back(point.detrended_price_oscillator_20);
        hurst_values.push_back(point.hurst_exponent_100);
        garch_vol_values.push_back(point.garch_volatility_21);
    }
    
    // Statistical Normalization
    if (ImPlot::BeginPlot(("Statistical Measures - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("Z-Score 20", indices.data(), z_score_values.data(), indices.size());
        ImPlot::PlotLine("Percentile Rank 50", indices.data(), percentile_rank_values.data(), indices.size());
        ImPlot::PlotLine("Coeff of Variation", indices.data(), coeff_var_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Advanced Statistical Indicators
    if (ImPlot::BeginPlot(("Advanced Statistics - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("DPO", indices.data(), dpo_values.data(), indices.size());
        ImPlot::PlotLine("Hurst Exponent", indices.data(), hurst_values.data(), indices.size());
        ImPlot::PlotLine("GARCH Volatility", indices.data(), garch_vol_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderRiskMetrics(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> cvar_values, drawdown_values, ulcer_values, sortino_values;
    std::vector<float> adx_values, poly_slope_values;
    
    for (const auto& point : data) {
        cvar_values.push_back(point.conditional_value_at_risk_cvar_95_20);
        drawdown_values.push_back(point.drawdown_duration_from_peak_50);
        ulcer_values.push_back(point.ulcer_index_14);
        sortino_values.push_back(point.sortino_ratio_30);
        adx_values.push_back(point.adx_rating_14);
        poly_slope_values.push_back(point.polynomial_regression_price_degree_2_slope);
    }
    
    // Risk Measures
    if (ImPlot::BeginPlot(("Risk Metrics - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Value");
        ImPlot::PlotLine("CVaR 95%", indices.data(), cvar_values.data(), indices.size());
        ImPlot::PlotLine("Drawdown Duration", indices.data(), drawdown_values.data(), indices.size());
        ImPlot::PlotLine("Ulcer Index", indices.data(), ulcer_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Performance Ratios
    if (ImPlot::BeginPlot(("Performance Ratios - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Ratio");
        ImPlot::PlotLine("Sortino Ratio", indices.data(), sortino_values.data(), indices.size());
        ImPlot::PlotLine("ADX Rating", indices.data(), adx_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Polynomial Trend
    if (ImPlot::BeginPlot(("Polynomial Trend - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Slope");
        ImPlot::PlotLine("Poly Regression Slope", indices.data(), poly_slope_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderRegimeAnalysis(const std::string& symbol, const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    std::vector<float> indices = createIndices(data.size());
    std::vector<float> markov_regime_values, hmm_regime_values, chow_test_values;
    std::vector<float> high_vol_indicator_values, return_vol_interaction_values;
    std::vector<float> vol_rsi_interaction_values, price_kama_ratio_values;
    
    for (const auto& point : data) {
        markov_regime_values.push_back(point.markov_regime_switching_garch_2_state);
        hmm_regime_values.push_back(point.market_regime_hmm_3_states_price_vol);
        chow_test_values.push_back(point.chow_test_statistic_breakpoint_detection_50);
        high_vol_indicator_values.push_back(point.high_volatility_indicator_garch_threshold);
        return_vol_interaction_values.push_back(point.return_x_volume_interaction_10);
        vol_rsi_interaction_values.push_back(point.volatility_x_rsi_interaction_14);
        price_kama_ratio_values.push_back(point.price_to_kama_ratio_20_10_30);
    }
    
    // Regime Detection
    if (ImPlot::BeginPlot(("Market Regimes - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Regime State");
        ImPlot::PlotLine("Markov Regime", indices.data(), markov_regime_values.data(), indices.size());
        ImPlot::PlotLine("HMM Regime", indices.data(), hmm_regime_values.data(), indices.size());
        ImPlot::PlotLine("High Vol Indicator", indices.data(), high_vol_indicator_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Structural Break Detection
    if (ImPlot::BeginPlot(("Structural Breaks - " + symbol).c_str(), ImVec2(-1, 150))) {
        ImPlot::SetupAxes("Time Index", "Test Statistic");
        ImPlot::PlotLine("Chow Test", indices.data(), chow_test_values.data(), indices.size());
        ImPlot::EndPlot();
    }
    
    // Feature Interactions
    if (ImPlot::BeginPlot(("Feature Interactions - " + symbol).c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("Time Index", "Interaction Value");
        ImPlot::PlotLine("Return x Volume", indices.data(), return_vol_interaction_values.data(), indices.size());
        ImPlot::PlotLine("Volatility x RSI", indices.data(), vol_rsi_interaction_values.data(), indices.size());
        ImPlot::PlotLine("Price/KAMA Ratio", indices.data(), price_kama_ratio_values.data(), indices.size());
        ImPlot::EndPlot();
    }
}

void ChartRenderer::renderStatistics(const std::vector<StockData>& data) {
    if (data.empty()) return;
    
    const auto& latest = data.back();
    
    ImGui::Columns(4, "StatsColumns");
    
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
    ImGui::Text("ADX Rating: %.2f", latest.adx_rating_14);
    ImGui::Text("Money Flow: %.2f", latest.money_flow_index_14);
    
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
    ImGui::Text("Hurst Exp: %.4f", latest.hurst_exponent_100);
    ImGui::Text("GARCH Vol: %.4f", latest.garch_volatility_21);
    
    ImGui::NextColumn();
    
    // Column 4: Risk & Regime
    ImGui::Text("RISK & REGIME");
    ImGui::Separator();
    ImGui::Text("CVaR 95%%: %.4f", latest.conditional_value_at_risk_cvar_95_20);
    ImGui::Text("Sortino: %.4f", latest.sortino_ratio_30);
    ImGui::Text("Ulcer Index: %.4f", latest.ulcer_index_14);
    ImGui::Text("Drawdown: %.0f", latest.drawdown_duration_from_peak_50);
    ImGui::Text("Markov Regime: %.0f", latest.markov_regime_switching_garch_2_state);
    ImGui::Text("HMM Regime: %.0f", latest.market_regime_hmm_3_states_price_vol);
    ImGui::Text("High Vol: %.0f", latest.high_volatility_indicator_garch_threshold);
    
    ImGui::Columns(1);
    ImGui::Separator();
    
    // Additional metrics in rows
    ImGui::Text("DISTRIBUTION & PATTERNS");
    ImGui::Text("Skewness (30): %.4f  |  Kurtosis (30): %.4f  |  Z-Score: %.4f  |  Percentile Rank: %.2f", 
                latest.skewness_30, latest.kurtosis_30, latest.z_score_20, latest.percentile_rank_50);
    ImGui::Text("Candle Way: %.4f  |  Candle Filling: %.4f  |  Candle Amplitude: %.4f", 
                latest.candle_way, latest.candle_filling, latest.candle_amplitude);
    
    ImGui::Separator();
    ImGui::Text("VOLUME PROFILE & VWAP");
    ImGui::Text("VWAP: $%.4f  |  VWAP Dev: %.4f  |  HVN: $%.4f  |  LVN: $%.4f", 
                latest.volume_weighted_average_price_intraday, latest.vwap_deviation_stddev_30,
                latest.volume_profile_high_volume_node_intraday, latest.volume_profile_low_volume_node_intraday);
    
    ImGui::Separator();
    ImGui::Text("OSCILLATORS");
    ImGui::Text("Chande Mom: %.2f  |  Aroon: %.2f  |  TRIX: %.4f  |  Fisher: %.4f  |  Vortex: %.4f", 
                latest.chande_momentum_oscillator_14, latest.aroon_oscillator_25, latest.trix_15,
                latest.fisher_transform_10, latest.vortex_indicator_14);
}
