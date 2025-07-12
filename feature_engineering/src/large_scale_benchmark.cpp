#include "../include/neon_technical_indicators.h"
#include "../include/simd_technical_indicators.h"
#include "../include/technical_indicators.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

class LargeScaleBenchmark {
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
    void run_large_scale_benchmark() {
        std::cout << "\n=== LARGE SCALE DATA BENCHMARK ===" << std::endl;
        std::cout << "Finding optimal data sizes for SIMD performance" << std::endl;
        
        // Test much larger data sizes to find SIMD sweet spot
        std::vector<size_t> data_sizes = {
            100000,    // 100K
            500000,    // 500K  
            1000000,   // 1M
            2000000,   // 2M
            5000000,   // 5M
            10000000   // 10M
        };
        
        std::vector<size_t> iterations = {10, 5, 3, 2, 1, 1}; // Fewer iterations for massive datasets
        
        for (size_t i = 0; i < data_sizes.size(); ++i) {
            size_t data_size = data_sizes[i];
            size_t num_iterations = iterations[i];
            
            std::cout << "\n--- Data Size: " << data_size << " points (" 
                      << (data_size / 1000000.0) << "M), Iterations: " << num_iterations << " ---" << std::endl;
            
            // Generate test data
            auto prices = generate_test_data(data_size);
            auto high = generate_test_data(data_size, 102.0);
            auto low = generate_test_data(data_size, 98.0);
            
            benchmark_operation("Returns", prices, num_iterations, 
                [](const auto& data) { return TechnicalIndicators::calculate_returns(data); },
                [](const auto& data) { return NEONTechnicalIndicators::calculate_returns_neon(data); },
                2 * (data_size - 1));
            
            benchmark_operation("Moving Average", prices, num_iterations,
                [](const auto& data) { return TechnicalIndicators::simple_moving_average(data, 20); },
                [](const auto& data) { return NEONTechnicalIndicators::simple_moving_average_neon(data, 20); },
                (data_size - 20 + 1) * 21);
            
            benchmark_operation("Spread", high, low, num_iterations,
                [](const auto& h, const auto& l) { return TechnicalIndicators::compute_spread(h, l); },
                [](const auto& h, const auto& l) { return NEONTechnicalIndicators::compute_spread_neon(h, l); },
                data_size);
        }
        
        // Test massive batch processing
        benchmark_massive_batch_processing();
    }

private:
    template<typename ScalarFunc, typename NeonFunc>
    void benchmark_operation(const std::string& name, const std::vector<double>& data, 
                           size_t iterations, ScalarFunc scalar_func, NeonFunc neon_func, size_t ops_per_iter) {
        
        size_t total_ops = ops_per_iter * iterations;
        
        // Scalar benchmark
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = scalar_func(data);
                (void)result;
            }
        });
        
        // NEON benchmark
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = neon_func(data);
                    (void)result;
                }
            });
        }
        
        std::cout << name << ":" << std::endl;
        std::cout << "  Scalar: " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:   " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    template<typename ScalarFunc, typename NeonFunc>
    void benchmark_operation(const std::string& name, const std::vector<double>& data1, 
                           const std::vector<double>& data2, size_t iterations, 
                           ScalarFunc scalar_func, NeonFunc neon_func, size_t ops_per_iter) {
        
        size_t total_ops = ops_per_iter * iterations;
        
        // Scalar benchmark
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = scalar_func(data1, data2);
                (void)result;
            }
        });
        
        // NEON benchmark
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = neon_func(data1, data2);
                    (void)result;
                }
            });
        }
        
        std::cout << name << ":" << std::endl;
        std::cout << "  Scalar: " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:   " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_massive_batch_processing() {
        std::cout << "\n=== MASSIVE BATCH PROCESSING BENCHMARK ===" << std::endl;
        
        // Test processing many stocks simultaneously
        std::vector<size_t> num_stocks = {1000, 5000, 10000, 20000};
        const size_t points_per_stock = 1000;
        
        for (size_t stock_count : num_stocks) {
            std::cout << "\n--- Processing " << stock_count << " stocks with " 
                      << points_per_stock << " points each ---" << std::endl;
            
            // Generate multiple stock datasets
            std::vector<std::vector<double>> stock_data;
            stock_data.reserve(stock_count);
            for (size_t i = 0; i < stock_count; ++i) {
                stock_data.push_back(generate_test_data(points_per_stock));
            }
            
            // Sequential processing
            double sequential_time = measure_time_ms([&]() {
                for (const auto& stock : stock_data) {
                    auto returns = TechnicalIndicators::calculate_returns(stock);
                    auto sma = TechnicalIndicators::simple_moving_average(stock, 20);
                    (void)returns; (void)sma;
                }
            });
            
            // Parallel NEON processing
            double parallel_time = measure_time_ms([&]() {
                std::vector<std::vector<double>> results;
                NEONTechnicalIndicators::process_multiple_series_parallel_optimized(
                    stock_data, results,
                    [](const std::vector<double>& data) {
                        auto returns = NEONTechnicalIndicators::calculate_returns_neon(data);
                        auto sma = NEONTechnicalIndicators::simple_moving_average_neon(data, 20);
                        return returns; // Return one for simplicity
                    }
                );
            });
            
            double speedup = sequential_time / parallel_time;
            size_t total_ops = stock_count * points_per_stock * 2; // Returns + SMA operations
            
            std::cout << "  Sequential: " << std::fixed << std::setprecision(3) << sequential_time << " ms, "
                      << std::setprecision(3) << calculate_gflops(total_ops, sequential_time) << " GFLOPS" << std::endl;
            std::cout << "  Parallel:   " << std::fixed << std::setprecision(3) << parallel_time << " ms, "
                      << std::setprecision(3) << calculate_gflops(total_ops, parallel_time) << " GFLOPS" << std::endl;
            std::cout << "  Speedup:    " << std::fixed << std::setprecision(1) << speedup << "x" << std::endl;
            
            // Calculate theoretical peak performance
            double peak_gflops = calculate_gflops(total_ops, parallel_time);
            std::cout << "  Peak Performance: " << std::fixed << std::setprecision(1) << peak_gflops << " GFLOPS" << std::endl;
        }
    }
};

// Function to run large scale benchmark
void run_large_scale_benchmark() {
    LargeScaleBenchmark benchmark;
    benchmark.run_large_scale_benchmark();
}
