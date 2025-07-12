#include "../include/neon_technical_indicators.h"
#include "../include/simd_technical_indicators.h"
#include "../include/technical_indicators.h"
#include "../include/batch_ohlc_processor.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <thread>

class PerformanceBenchmark {
private:
    std::vector<double> generate_test_data(size_t size, double base_price = 100.0) {
        std::vector<double> data;
        data.reserve(size);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, 0.02); // 2% daily volatility
        
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
    void run_comprehensive_benchmark() {
        std::cout << "\n=== COMPREHENSIVE PERFORMANCE BENCHMARK ===" << std::endl;
        std::cout << "Testing optimized NEON vs AVX2 vs Scalar implementations" << std::endl;
        
        // Test different data sizes
        std::vector<size_t> data_sizes = {1000, 5000, 10000, 50000, 100000};
        std::vector<size_t> iterations = {1000, 200, 100, 20, 10}; // Fewer iterations for larger datasets
        
        for (size_t i = 0; i < data_sizes.size(); ++i) {
            size_t data_size = data_sizes[i];
            size_t num_iterations = iterations[i];
            
            std::cout << "\n--- Data Size: " << data_size << " points, Iterations: " << num_iterations << " ---" << std::endl;
            
            // Generate test data
            auto prices = generate_test_data(data_size);
            auto high = generate_test_data(data_size, 102.0);
            auto low = generate_test_data(data_size, 98.0);
            
            benchmark_returns(prices, num_iterations);
            benchmark_moving_average(prices, num_iterations);
            benchmark_volatility(prices, num_iterations);
            benchmark_linear_slope(prices, num_iterations);
            benchmark_spread(high, low, num_iterations);
        }
        
        // Multi-threaded benchmark
        benchmark_parallel_processing();
        
        // Memory bandwidth test
        benchmark_memory_bandwidth();
    }

private:
    void benchmark_returns(const std::vector<double>& prices, size_t iterations) {
        std::cout << "\nReturns Calculation:" << std::endl;
        
        // Estimate operations: (N-1) subtractions + (N-1) divisions = 2*(N-1) ops
        size_t ops_per_iteration = 2 * (prices.size() - 1);
        size_t total_ops = ops_per_iteration * iterations;
        
        // Scalar benchmark
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = TechnicalIndicators::calculate_returns(prices);
                (void)result; // Prevent optimization
            }
        });
        
        // NEON benchmark
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = NEONTechnicalIndicators::calculate_returns_neon(prices);
                    (void)result;
                }
            });
        }
        
        // AVX2 benchmark
        double avx2_time = 0.0;
        if (SIMDTechnicalIndicators::is_simd_available()) {
            avx2_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = SIMDTechnicalIndicators::calculate_returns_simd(prices);
                    (void)result;
                }
            });
        }
        
        std::cout << "  Scalar:  " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:    " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
        
        if (avx2_time > 0) {
            double speedup = scalar_time / avx2_time;
            std::cout << "  AVX2:    " << std::fixed << std::setprecision(3) << avx2_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, avx2_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_moving_average(const std::vector<double>& prices, size_t iterations) {
        std::cout << "\nSimple Moving Average (20-period):" << std::endl;
        
        const size_t window = 20;
        // Estimate operations: rolling sum + division for each window
        size_t ops_per_iteration = (prices.size() - window + 1) * (window + 1);
        size_t total_ops = ops_per_iteration * iterations;
        
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = TechnicalIndicators::simple_moving_average(prices, window);
                (void)result;
            }
        });
        
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = NEONTechnicalIndicators::simple_moving_average_neon(prices, window);
                    (void)result;
                }
            });
        }
        
        double avx2_time = 0.0;
        if (SIMDTechnicalIndicators::is_simd_available()) {
            avx2_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = SIMDTechnicalIndicators::simple_moving_average_simd(prices, window);
                    (void)result;
                }
            });
        }
        
        std::cout << "  Scalar:  " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:    " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
        
        if (avx2_time > 0) {
            double speedup = scalar_time / avx2_time;
            std::cout << "  AVX2:    " << std::fixed << std::setprecision(3) << avx2_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, avx2_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_volatility(const std::vector<double>& prices, size_t iterations) {
        std::cout << "\nRolling Volatility (20-period):" << std::endl;
        
        // First calculate returns
        auto returns = TechnicalIndicators::calculate_returns(prices);
        if (returns.empty()) return;
        
        const size_t window = 20;
        // Estimate operations: mean calculation + variance calculation + sqrt
        size_t ops_per_iteration = (returns.size() - window + 1) * (window * 3 + 1);
        size_t total_ops = ops_per_iteration * iterations;
        
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = TechnicalIndicators::calculate_rolling_volatility(returns, window);
                (void)result;
            }
        });
        
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = NEONTechnicalIndicators::calculate_rolling_volatility_neon(returns, window);
                    (void)result;
                }
            });
        }
        
        double avx2_time = 0.0;
        if (SIMDTechnicalIndicators::is_simd_available()) {
            avx2_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = SIMDTechnicalIndicators::calculate_rolling_volatility_simd(returns, window);
                    (void)result;
                }
            });
        }
        
        std::cout << "  Scalar:  " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:    " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
        
        if (avx2_time > 0) {
            double speedup = scalar_time / avx2_time;
            std::cout << "  AVX2:    " << std::fixed << std::setprecision(3) << avx2_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, avx2_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_linear_slope(const std::vector<double>& prices, size_t iterations) {
        std::cout << "\nLinear Slope (20-period):" << std::endl;
        
        const size_t window = 20;
        // Estimate operations: sum calculations + dot product + arithmetic
        size_t ops_per_iteration = (prices.size() - window + 1) * (window * 2 + 5);
        size_t total_ops = ops_per_iteration * iterations;
        
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = TechnicalIndicators::linear_slope(prices, window);
                (void)result;
            }
        });
        
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = NEONTechnicalIndicators::linear_slope_neon(prices, window);
                    (void)result;
                }
            });
        }
        
        double avx2_time = 0.0;
        if (SIMDTechnicalIndicators::is_simd_available()) {
            avx2_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = SIMDTechnicalIndicators::linear_slope_simd(prices, window);
                    (void)result;
                }
            });
        }
        
        std::cout << "  Scalar:  " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:    " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
        
        if (avx2_time > 0) {
            double speedup = scalar_time / avx2_time;
            std::cout << "  AVX2:    " << std::fixed << std::setprecision(3) << avx2_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, avx2_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_spread(const std::vector<double>& high, const std::vector<double>& low, size_t iterations) {
        std::cout << "\nSpread Calculation:" << std::endl;
        
        size_t ops_per_iteration = high.size(); // One subtraction per element
        size_t total_ops = ops_per_iteration * iterations;
        
        double scalar_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = TechnicalIndicators::compute_spread(high, low);
                (void)result;
            }
        });
        
        double neon_time = 0.0;
        if (NEONTechnicalIndicators::is_neon_available()) {
            neon_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = NEONTechnicalIndicators::compute_spread_neon(high, low);
                    (void)result;
                }
            });
        }
        
        double avx2_time = 0.0;
        if (SIMDTechnicalIndicators::is_simd_available()) {
            avx2_time = measure_time_ms([&]() {
                for (size_t i = 0; i < iterations; ++i) {
                    auto result = SIMDTechnicalIndicators::compute_spread_simd(high, low);
                    (void)result;
                }
            });
        }
        
        std::cout << "  Scalar:  " << std::fixed << std::setprecision(3) << scalar_time << " ms, " 
                  << std::setprecision(3) << calculate_gflops(total_ops, scalar_time) << " GFLOPS" << std::endl;
        
        if (neon_time > 0) {
            double speedup = scalar_time / neon_time;
            std::cout << "  NEON:    " << std::fixed << std::setprecision(3) << neon_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, neon_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
        
        if (avx2_time > 0) {
            double speedup = scalar_time / avx2_time;
            std::cout << "  AVX2:    " << std::fixed << std::setprecision(3) << avx2_time << " ms, " 
                      << std::setprecision(3) << calculate_gflops(total_ops, avx2_time) << " GFLOPS, " 
                      << std::setprecision(1) << speedup << "x speedup" << std::endl;
        }
    }
    
    void benchmark_parallel_processing() {
        std::cout << "\n--- Parallel Processing Benchmark ---" << std::endl;
        
        const size_t num_series = 1000;
        const size_t series_length = 1000;
        
        // Generate multiple time series
        std::vector<std::vector<double>> series_data;
        series_data.reserve(num_series);
        for (size_t i = 0; i < num_series; ++i) {
            series_data.push_back(generate_test_data(series_length));
        }
        
        std::cout << "Processing " << num_series << " series of " << series_length << " points each" << std::endl;
        
        // Sequential processing
        double sequential_time = measure_time_ms([&]() {
            for (const auto& series : series_data) {
                auto result = TechnicalIndicators::calculate_returns(series);
                (void)result;
            }
        });
        
        // Parallel processing
        double parallel_time = measure_time_ms([&]() {
            std::vector<std::vector<double>> results;
            if (NEONTechnicalIndicators::is_neon_available()) {
                NEONTechnicalIndicators::process_multiple_series_parallel_optimized(
                    series_data, results, 
                    [](const std::vector<double>& data) {
                        return NEONTechnicalIndicators::calculate_returns_neon(data);
                    }
                );
            } else {
                // Fallback to manual parallel processing
                results.resize(series_data.size());
                const size_t num_threads = std::thread::hardware_concurrency();
                const size_t chunk_size = (series_data.size() + num_threads - 1) / num_threads;
                
                std::vector<std::thread> threads;
                for (size_t t = 0; t < num_threads; ++t) {
                    threads.emplace_back([&, t]() {
                        const size_t start_idx = t * chunk_size;
                        const size_t end_idx = std::min(start_idx + chunk_size, series_data.size());
                        
                        for (size_t i = start_idx; i < end_idx; ++i) {
                            results[i] = TechnicalIndicators::calculate_returns(series_data[i]);
                        }
                    });
                }
                
                for (auto& thread : threads) {
                    thread.join();
                }
            }
        });
        
        double speedup = sequential_time / parallel_time;
        std::cout << "  Sequential: " << std::fixed << std::setprecision(3) << sequential_time << " ms" << std::endl;
        std::cout << "  Parallel:   " << std::fixed << std::setprecision(3) << parallel_time << " ms" << std::endl;
        std::cout << "  Speedup:    " << std::fixed << std::setprecision(1) << speedup << "x" << std::endl;
    }
    
    void benchmark_memory_bandwidth() {
        std::cout << "\n--- Memory Bandwidth Test ---" << std::endl;
        
        const size_t data_size = 10000000; // 10M elements
        const size_t iterations = 10;
        
        auto data = generate_test_data(data_size);
        
        std::cout << "Testing memory bandwidth with " << data_size << " elements" << std::endl;
        
        // Simple copy operation to measure memory bandwidth
        double copy_time = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                std::vector<double> copy(data);
                (void)copy;
            }
        });
        
        // Calculate bandwidth in GB/s
        double bytes_transferred = static_cast<double>(data_size * sizeof(double) * iterations * 2); // Read + Write
        double time_seconds = copy_time / 1000.0;
        
        if (time_seconds > 0.001) { // Only calculate if time is meaningful (> 1ms)
            double bandwidth_gbs = bytes_transferred / time_seconds / (1024.0 * 1024.0 * 1024.0);
            std::cout << "  Memory Bandwidth: " << std::fixed << std::setprecision(2) << bandwidth_gbs << " GB/s" << std::endl;
        } else {
            std::cout << "  Memory Bandwidth: Too fast to measure accurately (< 1ms)" << std::endl;
        }
    }
};

// Function to run benchmark from main
void run_performance_benchmark() {
    PerformanceBenchmark benchmark;
    benchmark.run_comprehensive_benchmark();
}
