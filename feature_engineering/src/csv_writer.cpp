#include "../include/csv_writer.h"
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>

std::string FastCSVWriter::format_datetime(const std::chrono::system_clock::time_point& tp) {
    if (tp == std::chrono::system_clock::time_point::min()) return "";
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
    #ifdef _WIN32
    localtime_s(&tm, &time_t);
    #else
    localtime_r(&time_t, &tm);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string FastCSVWriter::format_double(double value, int precision) {
    if (std::isnan(value) || std::isinf(value)) return "";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string FastCSVWriter::safe_get_value(const std::vector<double>& vec, size_t index, size_t offset) {
    size_t effective_index = index >= offset ? index - offset : -1;
    return (effective_index < vec.size()) ? format_double(vec[effective_index]) : "";
}

std::string FastCSVWriter::safe_get_int_value(const std::vector<int>& vec, size_t index, size_t offset) {
    size_t effective_index = index >= offset ? index - offset : -1;
    return (effective_index < vec.size()) ? std::to_string(vec[effective_index]) : "";
}

void FastCSVWriter::write_ohlcv_with_features(
    const std::string& filepath, const OHLCVData& ohlcv_data,
    const FeatureSet& features, const std::string& data_frequency) {
    try {
        if (auto p = std::filesystem::path(filepath).parent_path(); !p.empty()) {
            std::filesystem::create_directories(p);
        }
        
        // Use large buffer for maximum write performance
        constexpr size_t BUFFER_SIZE = 1024 * 1024; // 1MB buffer
        std::vector<char> buffer;
        buffer.reserve(ohlcv_data.size() * 200); // Estimate line length
        
        // Build entire CSV content in memory first
        std::string content;
        content.reserve(ohlcv_data.size() * 200);
        
        // Header
        content += "datetime,open,high,low,close,volume,symbol,data_frequency,returns,sma,rsi,volatility,momentum,spread,internal_bar_strength,skewness_30,kurtosis_30,log_pct_change_5,auto_correlation_50_10,kama_10_2_30,linear_slope_20,linear_slope_60,parkinson_volatility_20,volume_sma_20,velocity,acceleration,candle_way,candle_filling,candle_amplitude,";
        content += "z_score_20,percentile_rank_50,coefficient_of_variation_30,detrended_price_oscillator_20,hurst_exponent_100,garch_volatility_21,shannon_entropy_volume_10,";
        content += "chande_momentum_oscillator_14,aroon_oscillator_25,trix_15,vortex_indicator_14,supertrend_10_3,ichimoku_senkou_span_A_9_26,ichimoku_senkou_span_B_26_52,fisher_transform_10,";
        content += "volume_weighted_average_price_intraday,volume_profile_high_volume_node_intraday,volume_profile_low_volume_node_intraday,on_balance_volume_sma_20,klinger_oscillator_34_55,money_flow_index_14,vwap_deviation_stddev_30,";
        content += "markov_regime_switching_garch_2_state,adx_rating_14,chow_test_statistic_breakpoint_detection_50,market_regime_hmm_3_states_price_vol,high_volatility_indicator_garch_threshold,";
        content += "return_x_volume_interaction_10,volatility_x_rsi_interaction_14,price_to_kama_ratio_20_10_30,polynomial_regression_price_degree_2_slope,";
        content += "conditional_value_at_risk_cvar_95_20,drawdown_duration_from_peak_50,ulcer_index_14,sortino_ratio_30\n";
        
        // Pre-allocate string for reuse
        std::string line;
        line.reserve(500); // Increased for more features
        
        for (size_t i = 0; i < ohlcv_data.size(); ++i) {
            line.clear();
            
            // Build line efficiently - Original features
            line += format_datetime(ohlcv_data.timestamps[i]) + ",";
            line += format_double(ohlcv_data.open[i]) + ",";
            line += format_double(ohlcv_data.high[i]) + ",";
            line += format_double(ohlcv_data.low[i]) + ",";
            line += format_double(ohlcv_data.close[i]) + ",";
            line += format_double(ohlcv_data.volume[i], 0) + ",";
            line += ohlcv_data.symbol + "," + data_frequency + ",";
            line += safe_get_value(features.returns, i, 1) + ",";
            line += safe_get_value(features.sma, i, 19) + ",";
            line += safe_get_value(features.rsi, i, 14) + ",";
            line += safe_get_value(features.volatility, i, 20) + ",";
            line += safe_get_value(features.momentum, i, 10) + ",";
            line += safe_get_value(features.spread, i, 0) + ",";
            line += safe_get_value(features.internal_bar_strength, i, 0) + ",";
            line += safe_get_value(features.skewness_30, i, 29) + ",";
            line += safe_get_value(features.kurtosis_30, i, 29) + ",";
            line += safe_get_value(features.log_pct_change_5, i, 5) + ",";
            line += safe_get_value(features.auto_correlation_50_10, i, 59) + ",";
            line += safe_get_value(features.kama_10_2_30, i, 0) + ",";
            line += safe_get_value(features.linear_slope_20, i, 19) + ",";
            line += safe_get_value(features.linear_slope_60, i, 59) + ",";
            line += safe_get_value(features.parkinson_volatility_20, i, 19) + ",";
            line += safe_get_value(features.volume_sma_20, i, 19) + ",";
            line += safe_get_value(features.velocity, i, 1) + ",";
            line += safe_get_value(features.acceleration, i, 2) + ",";
            line += safe_get_int_value(features.candle_way, i, 0) + ",";
            line += safe_get_value(features.candle_filling, i, 0) + ",";
            line += safe_get_value(features.candle_amplitude, i, 0) + ",";
            
            // Statistical/Mathematical Features
            line += safe_get_value(features.z_score_20, i, 19) + ",";
            line += safe_get_value(features.percentile_rank_50, i, 49) + ",";
            line += safe_get_value(features.coefficient_of_variation_30, i, 29) + ",";
            line += safe_get_value(features.detrended_price_oscillator_20, i, 19) + ",";
            line += safe_get_value(features.hurst_exponent_100, i, 99) + ",";
            line += safe_get_value(features.garch_volatility_21, i, 20) + ",";
            line += safe_get_value(features.shannon_entropy_volume_10, i, 9) + ",";
            
            // Technical Analysis Extended
            line += safe_get_value(features.chande_momentum_oscillator_14, i, 14) + ",";
            line += safe_get_value(features.aroon_oscillator_25, i, 24) + ",";
            line += safe_get_value(features.trix_15, i, 45) + ","; // 3 EMAs of 15 periods each
            line += safe_get_value(features.vortex_indicator_14, i, 14) + ",";
            line += safe_get_value(features.supertrend_10_3, i, 9) + ",";
            line += safe_get_value(features.ichimoku_senkou_span_A_9_26, i, 25) + ",";
            line += safe_get_value(features.ichimoku_senkou_span_B_26_52, i, 51) + ",";
            line += safe_get_value(features.fisher_transform_10, i, 9) + ",";
            
            // Volume/Liquidity Advanced
            line += safe_get_value(features.volume_weighted_average_price_intraday, i, 0) + ",";
            line += safe_get_value(features.volume_profile_high_volume_node_intraday, i, 0) + ",";
            line += safe_get_value(features.volume_profile_low_volume_node_intraday, i, 0) + ",";
            line += safe_get_value(features.on_balance_volume_sma_20, i, 20) + ",";
            line += safe_get_value(features.klinger_oscillator_34_55, i, 55) + ",";
            line += safe_get_value(features.money_flow_index_14, i, 14) + ",";
            line += safe_get_value(features.vwap_deviation_stddev_30, i, 30) + ",";
            
            // Regime Detection
            line += safe_get_value(features.markov_regime_switching_garch_2_state, i, 0) + ",";
            line += safe_get_value(features.adx_rating_14, i, 14) + ",";
            line += safe_get_value(features.chow_test_statistic_breakpoint_detection_50, i, 100) + ",";
            line += safe_get_value(features.market_regime_hmm_3_states_price_vol, i, 0) + ",";
            line += safe_get_value(features.high_volatility_indicator_garch_threshold, i, 20) + ",";
            
            // Non-Linear/Interaction
            line += safe_get_value(features.return_x_volume_interaction_10, i, 10) + ",";
            line += safe_get_value(features.volatility_x_rsi_interaction_14, i, 0) + ",";
            line += safe_get_value(features.price_to_kama_ratio_20_10_30, i, 0) + ",";
            line += safe_get_value(features.polynomial_regression_price_degree_2_slope, i, 19) + ",";
            
            // Alternative Risk Measures
            line += safe_get_value(features.conditional_value_at_risk_cvar_95_20, i, 19) + ",";
            line += safe_get_value(features.drawdown_duration_from_peak_50, i, 49) + ",";
            line += safe_get_value(features.ulcer_index_14, i, 13) + ",";
            line += safe_get_value(features.sortino_ratio_30, i, 29) + "\n";
            
            content += line;
        }
        
        // Single write operation for maximum speed
        std::ofstream file(filepath, std::ios::out | std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("Cannot create file: " + filepath);
        
        // Set large buffer for file stream
        file.rdbuf()->pubsetbuf(nullptr, BUFFER_SIZE);
        file.write(content.data(), content.size());
        file.close();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error writing CSV file: " + std::string(e.what()));
    }
}
