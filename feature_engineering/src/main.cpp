#include "csv_reader.h"
#include "csv_writer.h"
#include "batch_ohlc_processor.h"
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>

void create_dummy_csv(const std::string& filepath, int num_rows) {
    std::ofstream file(filepath);
    file << "symbol,datetime,open,high,low,close,volume\n";
    double price = 100.0;
    for (int i = 0; i < num_rows; ++i) {
        price += (rand() % 100 - 49) / 100.0;
        file << "DUMMY,"
             << "2023-01-" << (i % 28 + 1) << " 10:00:00,"
             << price << "," << price + 0.5 << "," << price - 0.5 << "," << price + 0.1 << ","
             << (rand() % 1000 + 500) << "\n";
    }
}

int main() {
    const std::string input_dir = "/Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.11.25";
    const std::string output_dir = "/Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25";

    try {
        std::cout << "Reading data from: " << input_dir << std::endl;
        
        // Get all CSV files first
        std::vector<std::string> csv_files;
        for (const auto& entry : std::filesystem::directory_iterator(input_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                csv_files.push_back(entry.path().string());
            }
        }
        
        std::cout << "Found " << csv_files.size() << " CSV files. Reading in parallel..." << std::endl;
        
        // Parallel file reading for maximum I/O throughput
        const unsigned int num_read_threads = std::min(static_cast<unsigned int>(csv_files.size()), 
                                                      std::thread::hardware_concurrency());
        std::vector<std::unique_ptr<OHLCVData>> ohlcv_series(csv_files.size());
        std::vector<std::thread> read_threads;
        std::atomic<size_t> read_completed{0};
        std::mutex read_progress_mutex;
        
        const size_t read_batch_size = (csv_files.size() + num_read_threads - 1) / num_read_threads;
        
        auto read_start = std::chrono::high_resolution_clock::now();
        
        for (unsigned int t = 0; t < num_read_threads; ++t) {
            read_threads.emplace_back([&, t]() {
                const size_t start_idx = t * read_batch_size;
                const size_t end_idx = std::min(start_idx + read_batch_size, csv_files.size());
                
                for (size_t i = start_idx; i < end_idx; ++i) {
                    try {
                        ohlcv_series[i] = FastCSVReader::read_csv_file(csv_files[i]);
                        
                        size_t current_count = ++read_completed;
                        if (current_count % 500 == 0 || current_count == csv_files.size()) {
                            std::lock_guard<std::mutex> lock(read_progress_mutex);
                            std::cout << "  - Read progress: " << current_count << "/" << csv_files.size() 
                                     << " (" << (current_count * 100 / csv_files.size()) << "%)" << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::lock_guard<std::mutex> lock(read_progress_mutex);
                        std::cerr << "Error reading " << csv_files[i] << ": " << e.what() << std::endl;
                    }
                }
            });
        }
        
        // Wait for all read threads to complete
        for (auto& thread : read_threads) {
            thread.join();
        }
        
        auto read_end = std::chrono::high_resolution_clock::now();
        auto read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(read_end - read_start);
        
        // Remove null entries
        ohlcv_series.erase(
            std::remove_if(ohlcv_series.begin(), ohlcv_series.end(),
                          [](const auto& ptr) { return !ptr || ptr->empty(); }),
            ohlcv_series.end()
        );
        
        std::cout << "Parallel reading completed in " << read_duration.count() << " ms" << std::endl;
        std::cout << "Successfully loaded " << ohlcv_series.size() << " stock datasets." << std::endl;

        std::vector<std::vector<double>> opens, highs, lows, closes, volumes;
        opens.reserve(ohlcv_series.size());
        highs.reserve(ohlcv_series.size());
        lows.reserve(ohlcv_series.size());
        closes.reserve(ohlcv_series.size());
        volumes.reserve(ohlcv_series.size());
        
        for(const auto& data : ohlcv_series) {
            opens.push_back(data->open);
            highs.push_back(data->high);
            lows.push_back(data->low);
            closes.push_back(data->close);
            volumes.push_back(data->volume);
        }

        // Get number of CPU cores for optimal parallelization
        const unsigned int num_threads = std::thread::hardware_concurrency();
        std::cout << "Using " << num_threads << " threads for parallel processing" << std::endl;

        BatchOHLCProcessor processor;
        std::cout << "Calculating features for " << ohlcv_series.size() << " stocks..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Parallel feature calculation and file writing
        std::filesystem::create_directories(output_dir);
        std::atomic<size_t> completed_count{0};
        std::mutex progress_mutex;
        
        // Process stocks in parallel batches
        std::vector<std::thread> threads;
        const size_t batch_size = (ohlcv_series.size() + num_threads - 1) / num_threads;
        
        for (unsigned int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                const size_t start_idx = t * batch_size;
                const size_t end_idx = std::min(start_idx + batch_size, ohlcv_series.size());
                
                for (size_t i = start_idx; i < end_idx; ++i) {
                    try {
                        // Calculate features for this stock
                        auto features = processor.calculate_features(
                            opens[i], highs[i], lows[i], closes[i], volumes[i]
                        );
                        
                        // Write to file
                        const std::string output_path = output_dir + "/" + ohlcv_series[i]->symbol + "_features.csv";
                        FastCSVWriter::write_ohlcv_with_features(output_path, *ohlcv_series[i], features);
                        
                        // Update progress
                        size_t current_count = ++completed_count;
                        if (current_count % 100 == 0 || current_count == ohlcv_series.size()) {
                            std::lock_guard<std::mutex> lock(progress_mutex);
                            std::cout << "  - Progress: " << current_count << "/" << ohlcv_series.size() 
                                     << " (" << (current_count * 100 / ohlcv_series.size()) << "%)" << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::lock_guard<std::mutex> lock(progress_mutex);
                        std::cerr << "Error processing " << ohlcv_series[i]->symbol << ": " << e.what() << std::endl;
                    }
                }
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Parallel processing completed in " << duration.count() << " ms" << std::endl;
        std::cout << "Successfully processed " << completed_count.load() << " stocks!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
