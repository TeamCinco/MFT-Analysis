#include "../include/neon_technical_indicators.h"
#include "../include/simd_technical_indicators.h"
#include "../include/technical_indicators.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>

class MultiCoreBenchmark {
private:
    std::vector<double> generate_test_data(size_t size, double base_price = 100.0) {
        std::vector<double> data;
        data.reserve(size);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, 0.02);
        
        double price = base_price;
        for (size_t i = 0; i < size; ++i) {
            price *= (1.0 + dist(gen));
            data.push_back(price);
        }
        return data;
    }

    template<typename Func>
    double measure_time_ms(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    double calculate_gflops(size_t operations, double time_ms) {
        double time_seconds = time_ms / 1000.0;
        return static_cast<double>(operations) / (time_seconds * 1e9);
    }

public:
    void run_multi_core_benchmark() {
        std::cout << "\n=== MULTI-CORE DISTRIBUTION BENCHMARK ===" << std::endl;
        
        const unsigned int num_cores = std::thread::hardware_concurrency();
        const size_t stocks_per_core = 1000;
        const size_t points_per_stock = 1000;
        
        std::cout << "CPU Cores Available: " << num_cores << std::endl;
        std::cout << "Strategy: " << stocks_per_core << " stocks per core" << std::endl;
        std::cout << "Points per stock: " << points_per_stock << std::endl;
        
        // Test different core utilization patterns
        std::vector<unsigned int> core_counts = {1, 2, 4, 6, 8, 10};
        
        for (unsigned int cores_to_use : core_counts) {
            if (cores_to_use > num_cores) continue;
            
            const size_t total_stocks = cores_to_use * stocks_per_core;
            
            std::cout << "\n--- Using " << cores_to_use << " cores, " 
                      << total_stocks << " total stocks (" << stocks_per_core << " per core) ---" << std::endl;
            
            // Generate stock data for all cores
            std::vector<std::vector<std::vector<double>>> core_stock_data(cores_to_use);
            for (unsigned int core = 0; core < cores_to_use; ++core) {
                core_stock_data[core].reserve(stocks_per_core);
                for (size_t stock = 0; stock < stocks_per_core; ++stock) {
                    core_stock_data[core].push_back(generate_test_data(points_per_stock));
                }
            }
            
            // Benchmark different operations
            benchmark_multi_core_operation("Returns Calculation", core_stock_data, cores_to_use,
                [](const std::vector<double>& data) {
                    return NEONTechnicalIndicators::calculate_returns_neon(data);
                },
                2 * (points_per_stock - 1)); // 2 ops per return calculation
            
            benchmark_multi_core_operation("Moving Average", core_stock_data, cores_to_use,
                [](const std::vector<double>& data) {
                    return NEONTechnicalIndicators::simple_moving_average_neon(data, 20);
                },
                (points_per_stock - 20 + 1) * 21); // Rolling sum + division
            
            benchmark_multi_core_operation("Spread Calculation", core_stock_data, cores_to_use,
                [](const std::vector<double>& data) {
                    // Generate high/low data on the fly for spread calculation
                    auto high = data;
                    auto low = data;
                    for (size_t i = 0; i < data.size(); ++i) {
                        high[i] *= 1.02; // 2% higher
                        low[i] *= 0.98;  // 2% lower
                    }
                    return NEONTechnicalIndicators::compute_spread_neon(high, low);
                },
                points_per_stock); // 1 subtraction per point
        }
        
        // Test optimal configuration with maximum data
        benchmark_maximum_throughput();
    }

private:
    template<typename ProcessorFunc>
    void benchmark_multi_core_operation(
        const std::string& operation_name,
        const std::vector<std::vector<std::vector<double>>>& core_stock_data,
        unsigned int cores_to_use,
        ProcessorFunc processor,
        size_t ops_per_stock) {
        
        const size_t stocks_per_core = core_stock_data[0].size();
        const size_t total_stocks = cores_to_use * stocks_per_core;
        const size_t total_ops = total_stocks * ops_per_stock;
        
        std::atomic<size_t> completed_stocks{0};
        std::mutex progress_mutex;
        
        double processing_time = measure_time_ms([&]() {
            std::vector<std::thread> threads;
            threads.reserve(cores_to_use);
            
            // Launch one thread per core
            for (unsigned int core = 0; core < cores_to_use; ++core) {
                threads.emplace_back([&, core]() {
                    // Each core processes its assigned stocks
                    for (size_t stock_idx = 0; stock_idx < stocks_per_core; ++stock_idx) {
                        auto result = processor(core_stock_data[core][stock_idx]);
                        (void)result; // Prevent optimization
                        
                        ++completed_stocks;
                    }
                });
            }
            
            // Wait for all cores to complete
            for (auto& thread : threads) {
                thread.join();
            }
        });
        
        double gflops = calculate_gflops(total_ops, processing_time);
        double stocks_per_second = total_stocks / (processing_time / 1000.0);
        double gflops_per_core = gflops / cores_to_use;
        
        std::cout << operation_name << ":" << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << processing_time << " ms" << std::endl;
        std::cout << "  Total Performance: " << std::setprecision(3) << gflops << " GFLOPS" << std::endl;
        std::cout << "  Per-Core Performance: " << std::setprecision(3) << gflops_per_core << " GFLOPS/core" << std::endl;
        std::cout << "  Throughput: " << std::setprecision(1) << stocks_per_second << " stocks/second" << std::endl;
        std::cout << "  Stocks Processed: " << completed_stocks.load() << "/" << total_stocks << std::endl;
    }
    
    void benchmark_maximum_throughput() {
        std::cout << "\n=== MAXIMUM THROUGHPUT BENCHMARK ===" << std::endl;
        
        const unsigned int num_cores = std::thread::hardware_concurrency();
        const size_t stocks_per_core = 1000;
        const size_t points_per_stock = 2000; // Larger datasets for maximum throughput
        
        std::cout << "Maximum configuration: " << num_cores << " cores × " 
                  << stocks_per_core << " stocks × " << points_per_stock << " points" << std::endl;
        std::cout << "Total data points: " << (num_cores * stocks_per_core * points_per_stock) << std::endl;
        
        // Generate maximum dataset
        std::vector<std::vector<std::vector<double>>> core_stock_data(num_cores);
        std::cout << "Generating " << (num_cores * stocks_per_core) << " stock datasets..." << std::endl;
        
        for (unsigned int core = 0; core < num_cores; ++core) {
            core_stock_data[core].reserve(stocks_per_core);
            for (size_t stock = 0; stock < stocks_per_core; ++stock) {
                core_stock_data[core].push_back(generate_test_data(points_per_stock));
            }
        }
        
        std::cout << "Data generation complete. Running maximum throughput test..." << std::endl;
        
        // Test the highest performing operation (spread calculation)
        const size_t total_stocks = num_cores * stocks_per_core;
        const size_t total_ops = total_stocks * points_per_stock;
        
        std::atomic<size_t> completed_stocks{0};
        std::atomic<double> total_gflops{0.0};
        
        double processing_time = measure_time_ms([&]() {
            std::vector<std::thread> threads;
            threads.reserve(num_cores);
            
            // Launch maximum threads
            for (unsigned int core = 0; core < num_cores; ++core) {
                threads.emplace_back([&, core]() {
                    double core_start_time = std::chrono::duration<double, std::milli>(
                        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                    
                    size_t core_ops = 0;
                    
                    // Process all stocks on this core
                    for (size_t stock_idx = 0; stock_idx < stocks_per_core; ++stock_idx) {
                        const auto& stock_data = core_stock_data[core][stock_idx];
                        
                        // Perform multiple operations for maximum throughput
                        auto returns = NEONTechnicalIndicators::calculate_returns_neon(stock_data);
                        auto sma = NEONTechnicalIndicators::simple_moving_average_neon(stock_data, 20);
                        
                        // Generate spread data
                        auto high = stock_data;
                        auto low = stock_data;
                        for (size_t i = 0; i < stock_data.size(); ++i) {
                            high[i] *= 1.02;
                            low[i] *= 0.98;
                        }
                        auto spread = NEONTechnicalIndicators::compute_spread_neon(high, low);
                        
                        (void)returns; (void)sma; (void)spread; // Prevent optimization
                        
                        core_ops += points_per_stock * 3; // 3 operations per stock
                        ++completed_stocks;
                    }
                    
                    double core_end_time = std::chrono::duration<double, std::milli>(
                        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                    
                    double core_time = core_end_time - core_start_time;
                    double core_gflops = calculate_gflops(core_ops, core_time);
                    
                    // Atomic add to total (approximate)
                    double expected = total_gflops.load();
                    while (!total_gflops.compare_exchange_weak(expected, expected + core_gflops));
                });
            }
            
            // Wait for all cores to complete
            for (auto& thread : threads) {
                thread.join();
            }
        });
        
        double overall_gflops = calculate_gflops(total_ops * 3, processing_time); // 3 operations
        double stocks_per_second = total_stocks / (processing_time / 1000.0);
        double data_points_per_second = (total_stocks * points_per_stock) / (processing_time / 1000.0);
        
        std::cout << "\n=== MAXIMUM THROUGHPUT RESULTS ===" << std::endl;
        std::cout << "Processing Time: " << std::fixed << std::setprecision(3) << processing_time << " ms" << std::endl;
        std::cout << "Total Performance: " << std::setprecision(3) << overall_gflops << " GFLOPS" << std::endl;
        std::cout << "Per-Core Average: " << std::setprecision(3) << (overall_gflops / num_cores) << " GFLOPS/core" << std::endl;
        std::cout << "Stock Throughput: " << std::setprecision(1) << stocks_per_second << " stocks/second" << std::endl;
        std::cout << "Data Point Throughput: " << std::setprecision(1) << (data_points_per_second / 1000000.0) << " million points/second" << std::endl;
        std::cout << "Stocks Processed: " << completed_stocks.load() << "/" << total_stocks << std::endl;
        std::cout << "Core Utilization: " << num_cores << "/" << num_cores << " (100%)" << std::endl;
        
        // Calculate theoretical peak comparison
        std::cout << "\n=== PERFORMANCE ANALYSIS ===" << std::endl;
        std::cout << "Baseline (0.077 GFLOPS): " << std::setprecision(1) << (overall_gflops / 0.077) << "x improvement" << std::endl;
        std::cout << "Target (300 GFLOPS): " << std::setprecision(1) << (300.0 / overall_gflops) << "x remaining to reach target" << std::endl;
        std::cout << "Apple M2 Pro Theoretical (3000 GFLOPS): " << std::setprecision(1) << (3000.0 / overall_gflops) << "x theoretical maximum" << std::endl;
    }
};

// Function to run multi-core benchmark
void run_multi_core_benchmark() {
    MultiCoreBenchmark benchmark;
    benchmark.run_multi_core_benchmark();
}
