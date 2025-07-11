#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "../include/batch_processor.h"
#include "../include/technical_indicators.h"

PYBIND11_MODULE(ohlc_features, m) {
    m.doc() = "High-performance OHLC feature calculation library";
    
    pybind11::class_<BatchOHLCProcessor>(m, "BatchOHLCProcessor")
        .def(pybind11::init<>())
        
        // Main processing methods
        .def("batch_calculate_features", &BatchOHLCProcessor::batch_calculate_features)
        .def("calculate_stock_features", &BatchOHLCProcessor::calculate_stock_features)
        .def("calculate_comprehensive_features", &BatchOHLCProcessor::calculate_comprehensive_features)
        .def("batch_calculate_comprehensive_features", &BatchOHLCProcessor::batch_calculate_comprehensive_features)
        
        // Basic indicators
        .def("calculate_rsi", &BatchOHLCProcessor::calculate_rsi)
        .def("calculate_returns", &BatchOHLCProcessor::calculate_returns)
        .def("calculate_rolling_volatility", &BatchOHLCProcessor::calculate_rolling_volatility)
        .def("calculate_momentum", &BatchOHLCProcessor::calculate_momentum)
        
        // Advanced indicators
        .def("calculate_skewness", &BatchOHLCProcessor::calculate_skewness)
        .def("calculate_kurtosis", &BatchOHLCProcessor::calculate_kurtosis)
        .def("calculate_kama", &BatchOHLCProcessor::calculate_kama)
        .def("calculate_parkinson_vol", &BatchOHLCProcessor::calculate_parkinson_vol)
        .def("calculate_linear_slope", &BatchOHLCProcessor::calculate_linear_slope)
        .def("calculate_log_pct_change", &BatchOHLCProcessor::calculate_log_pct_change)
        .def("calculate_auto_correlation", &BatchOHLCProcessor::calculate_auto_correlation)
        
        // Candle indicators
        .def("calculate_spread", &BatchOHLCProcessor::calculate_spread)
        .def("calculate_internal_bar_strength", &BatchOHLCProcessor::calculate_internal_bar_strength);
    
    // Optional: Expose TechnicalIndicators directly for advanced users
    pybind11::class_<TechnicalIndicators>(m, "TechnicalIndicators")
        .def_static("calculate_returns", &TechnicalIndicators::calculate_returns)
        .def_static("calculate_rsi", &TechnicalIndicators::calculate_rsi)
        .def_static("simple_moving_average", &TechnicalIndicators::simple_moving_average)
        .def_static("compute_spread", &TechnicalIndicators::compute_spread)
        .def_static("internal_bar_strength", &TechnicalIndicators::internal_bar_strength)
        .def_static("candle_information", &TechnicalIndicators::candle_information)
        .def_static("derivatives", &TechnicalIndicators::derivatives)
        .def_static("log_pct_change", &TechnicalIndicators::log_pct_change)
        .def_static("auto_correlation", &TechnicalIndicators::auto_correlation)
        .def_static("skewness", &TechnicalIndicators::skewness)
        .def_static("kurtosis", &TechnicalIndicators::kurtosis)
        .def_static("kama", &TechnicalIndicators::kama)
        .def_static("linear_slope", &TechnicalIndicators::linear_slope)
        .def_static("close_to_close_volatility", &TechnicalIndicators::close_to_close_volatility)
        .def_static("parkinson_volatility", &TechnicalIndicators::parkinson_volatility);
}