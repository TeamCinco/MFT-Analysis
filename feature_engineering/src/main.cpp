#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>
#include <atomic>
#include "csv_reader.h"
#include "batch_processor.h"
#include "csv_writer.h"

int main(int argc, char* argv[]) {
    std::string data_directory = "/Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.11.25";
    std::string output_directory = "/Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25";
    
    if (argc > 1) data_directory = argv[1];
    if (argc > 2) output_directory = argv[2];
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        std::filesystem::create_directories(output_directory);
        auto all_data = FastCSVReader::read_directory(data_directory);
        
        std::atomic<size_t> processed{0};
        size_t total = all_data.size();
        
        std::cout << "Processing " << total << " files..." << std::endl;
        
        for (auto& data : all_data) {
            if (data && !data->empty()) {
                BatchOHLCProcessor processor;
                auto features = processor.calculate_organized_features(
                    data->open, data->high, data->low, data->close, data->volume
                );
                
                FastCSVWriter::write_ohlcv_with_features(
                    output_directory + "/" + data->symbol + "_features.csv",
                    *data, features.returns, features.sma, features.rsi,
                    features.volatility, features.momentum, features.spread,
                    features.internal_bar_strength, features.skewness_30,
                    features.kurtosis_30, features.log_pct_change_5,
                    features.auto_correlation_50_10, features.kama_10_2_30,
                    features.linear_slope_20, features.linear_slope_60,
                    features.parkinson_volatility_20, features.volume_sma_20,
                    features.velocity, features.acceleration, features.candle_way,
                    features.candle_filling, features.candle_amplitude, "daily"
                );
            }
            
            size_t current = processed.fetch_add(1) + 1;
            if (current % (total / 20 + 1) == 0 || current == total) {
                std::cout << "\r" << (current * 100 / total) << "%" << std::flush;
            }
        }
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::high_resolution_clock::now() - start
        );
        
        std::cout << "\nCompleted in " << duration.count() << " seconds" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}