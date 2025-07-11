#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <atomic>
#include <filesystem>
#include "csv_reader.h"
#include "batch_processor.h"
#include "csv_writer.h"

class PerformanceMonitor {
private:
    std::atomic<uint64_t> operations_count_{0};
    std::chrono::high_resolution_clock::time_point start_time_;
    
public:
    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        operations_count_ = 0;
    }
    
    void add_operations(uint64_t ops) {
        operations_count_.fetch_add(ops, std::memory_order_relaxed);
    }
    
    void print_stats() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
        
        uint64_t total_ops = operations_count_.load();
        double ops_per_second = static_cast<double>(total_ops) / (duration.count() / 1000.0);
        
        std::cout << "\n=== Performance Summary ===" << std::endl;
        std::cout << "Total operations: " << total_ops << std::endl;
        std::cout << "Duration: " << duration.count() << " ms" << std::endl;
        std::cout << "Operations per second: " << static_cast<uint64_t>(ops_per_second) << std::endl;
    }
};

void process_batch(std::vector<std::unique_ptr<OHLCVData>> batch, 
                   PerformanceMonitor& monitor,
                   const std::string& output_directory) {
    BatchOHLCProcessor processor;
    
    for (auto& data : batch) {
        // Calculate organized features using your existing method
        auto features = processor.calculate_organized_features(
            data->open, data->high, data->low, data->close, data->volume
        );
        
        // Save in organized format
        std::string output_path = output_directory + "/" + data->symbol + "_features.csv";
        try {
            FastCSVWriter::write_ohlcv_with_features(
                output_path,
                *data,
                features.returns,
                features.sma,
                features.rsi,
                features.volatility,
                features.momentum,
                features.spread,
                features.internal_bar_strength,
                features.skewness_30,
                features.kurtosis_30,
                features.log_pct_change_5,
                features.auto_correlation_50_10,
                features.kama_10_2_30,
                features.linear_slope_20,
                features.linear_slope_60,
                features.parkinson_volatility_20,
                features.volume_sma_20,
                features.velocity,
                features.acceleration,
                features.candle_way,
                features.candle_filling,
                features.candle_amplitude,
                "daily"
            );
            std::cout << "Saved features for " << data->symbol << " to " << output_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error saving " << data->symbol << ": " << e.what() << std::endl;
        }
        
        uint64_t ops = data->size() * 25; // ~25 operations per candle
        monitor.add_operations(ops);
    }
}

int main(int argc, char* argv[]) {
    std::string data_directory = "/Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.9.25";
    std::string output_directory = "/Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25";
    
    if (argc > 1) {
        data_directory = argv[1];
    }
    if (argc > 2) {
        output_directory = argv[2];
    }
    
    std::cout << "High-Performance OHLCV Feature Extraction" << std::endl;
    std::cout << "Data directory: " << data_directory << std::endl;
    std::cout << "Output directory: " << output_directory << std::endl;
    std::cout << "Hardware Threads: " << std::thread::hardware_concurrency() << std::endl;
    
    // Create output directory if it doesn't exist
    try {
        std::filesystem::create_directories(output_directory);
        std::cout << "Created output directory: " << output_directory << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error creating output directory: " << e.what() << std::endl;
        return 1;
    }
    
    PerformanceMonitor monitor;
    monitor.start();
    
    try {
        auto all_data = FastCSVReader::read_directory(data_directory);
        std::cout << "Loaded " << all_data.size() << " files" << std::endl;
        
        // Process in parallel
        const size_t num_threads = std::thread::hardware_concurrency();
        const size_t batch_size = (all_data.size() + num_threads - 1) / num_threads;
        
        std::vector<std::future<void>> futures;
        
        for (size_t i = 0; i < num_threads; ++i) {
            size_t start_idx = i * batch_size;
            size_t end_idx = std::min(start_idx + batch_size, all_data.size());
            
            if (start_idx < end_idx) {
                std::vector<std::unique_ptr<OHLCVData>> batch;
                for (size_t j = start_idx; j < end_idx; ++j) {
                    batch.push_back(std::move(all_data[j]));
                }
                
                futures.push_back(std::async(std::launch::async, 
                    process_batch, std::move(batch), std::ref(monitor), std::ref(output_directory)));
            }
        }
        
        for (auto& future : futures) {
            future.wait();
        }
        
        monitor.print_stats();
        
        std::cout << "\n=== Feature Extraction Complete ===" << std::endl;
        std::cout << "Results saved to: " << output_directory << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}