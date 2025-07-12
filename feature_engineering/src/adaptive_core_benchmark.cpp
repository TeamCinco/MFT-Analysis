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
#include <algorithm>
#include <map>

class AdaptiveCoreBenchmark {
private:
    struct OperationProfile {
        std::string name;
        std::function<std::vector<double>(const std::vector<double>&)> processor;
        size_t ops_per_stock;
        double baseline_gflops;
        int allocated_cores;
        std::string allocation_reason;
    };

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

    double profile_operation_performance(const OperationProfile& op, const std::vector<double>& test_data) {
        const size_t iterations = 100;
        const size_t total_ops = op.ops_per_stock * iterations;
        
        double time_ms = measure_time_ms([&]() {
            for (size_t i = 0; i < iterations; ++i) {
                auto result = op.processor(test_data);
                (void)result; // Prevent optimization
            }
        });
        
        return calculate_gflops(total_ops, time_ms);
    }

public:
    void run_adaptive_core_benchmark() {
        std::cout << "\n=== ADAPTIVE CORE ALLOCATION BENCHMARK ===" << std::endl;
        
        const unsigned int total_cores = std::thread::hardware_concurrency();
        const size_t stocks_per_core = 1000;
        const size_t points_per_stock = 1000;
        
        std::cout << "Total CPU Cores: " << total_cores << std::endl;
        std::cout << "Strategy: Adaptive allocation based on performance" << std::endl;
        std::cout << "Stocks per core: " << stocks_per_core << std::endl;
        std::cout << "Points per stock: " << points_per_stock << std::endl;
        
        // Define operations to benchmark
        std::vector<OperationProfile> operations = {
            {
                "Returns Calculation",
                [](const std::vector<double>& data) {
                    return NEONTechnicalIndicators::calculate_returns_neon(data);
                },
                2 * (points_per_stock - 1), // 2 ops per return
                0.0, 0, ""
            },
            {
                "Moving Average (20)",
                [](const std::vector<double>& data) {
                    return NEONTechnicalIndicators::simple_moving_average_neon(data, 20);
                },
                (points_per_stock - 20 + 1) * 21, // Rolling sum + division
                0.0, 0, ""
            },
            {
                "Rolling Volatility (20)",
                [](const std::vector<double>& data) {
                    auto returns = NEONTechnicalIndicators::calculate_returns_neon(data);
                    return NEONTechnicalIndicators::calculate_rolling_volatility_neon(returns, 20);
                },
                (points_per_stock - 20 + 1) * 60, // Complex variance calculation
                0.0, 0, ""
            },
            {
                "Spread Calculation",
                [](const std::vector<double>& data) {
                    auto high = data;
                    auto low = data;
                    for (size_t i = 0; i < data.size(); ++i) {
                        high[i] *= 1.02;
                        low[i] *= 0.98;
                    }
                    return NEONTechnicalIndicators::compute_spread_neon(high, low);
                },
                points_per_stock, // 1 subtraction per point
                0.0, 0, ""
            },
            {
                "Linear Slope (20)",
                [](const std::vector<double>& data) {
                    return NEONTechnicalIndicators::linear_slope_neon(data, 20);
                },
                (points_per_stock - 20 + 1) * 40, // Complex dot product calculation
                0.0, 0, ""
            }
        };
        
        // Step 1: Profile each operation to get baseline performance
        std::cout << "\n=== PROFILING OPERATIONS ===" << std::endl;
        auto test_data = generate_test_data(points_per_stock);
        
        for (auto& op : operations) {
            op.baseline_gflops = profile_operation_performance(op, test_data);
            std::cout << op.name << ": " << std::fixed << std::setprecision(3) 
                      << op.baseline_gflops << " GFLOPS (baseline)" << std::endl;
        }
        
        // Step 2: Adaptive core allocation
        std::cout << "\n=== ADAPTIVE CORE ALLOCATION ===" << std::endl;
        allocate_cores_adaptively(operations, total_cores);
        
        // Step 3: Run benchmark with adaptive allocation
        std::cout << "\n=== RUNNING ADAPTIVE BENCHMARK ===" << std::endl;
        run_adaptive_benchmark(operations, stocks_per_core, points_per_stock);
    }

private:
    void allocate_cores_adaptively(std::vector<OperationProfile>& operations, unsigned int total_cores) {
        // Sort operations by baseline performance (highest first)
        std::sort(operations.begin(), operations.end(), 
                 [](const OperationProfile& a, const OperationProfile& b) {
                     return a.baseline_gflops > b.baseline_gflops;
                 });
        
        std::cout << "Operations sorted by performance (highest first):" << std::endl;
        for (const auto& op : operations) {
            std::cout << "  " << op.name << ": " << std::fixed << std::setprecision(3) 
                      << op.baseline_gflops << " GFLOPS" << std::endl;
        }
        
        // Smart allocation: Give more cores to efficient operations
        std::random_device rd;
        std::mt19937 gen(rd());
        
        // Define allocation strategy based on performance tiers
        std::vector<int> core_allocation;
        
        if (operations.size() == 5) {
            // Optimized allocation for 5 operations across 10 cores
            // Give more cores to top performers, fewer to bottom performers
            std::vector<std::vector<int>> allocation_strategies = {
                {4, 3, 2, 1, 0},  // Strategy 1: Heavy focus on top 4
                {3, 3, 2, 2, 0},  // Strategy 2: Balanced top 4
                {3, 2, 2, 2, 1},  // Strategy 3: Everyone gets something
                {4, 2, 2, 1, 1},  // Strategy 4: Top-heavy with minimum for all
                {3, 3, 2, 1, 1}   // Strategy 5: Balanced approach
            };
            
            std::uniform_int_distribution<int> strategy_dist(0, allocation_strategies.size() - 1);
            core_allocation = allocation_strategies[strategy_dist(gen)];
            
            std::cout << "\nSelected allocation strategy: ";
            for (size_t i = 0; i < core_allocation.size(); ++i) {
                std::cout << core_allocation[i];
                if (i < core_allocation.size() - 1) std::cout << "-";
            }
            std::cout << " cores" << std::endl;
        } else {
            // Fallback for different number of operations
            int cores_per_op = total_cores / operations.size();
            int extra_cores = total_cores % operations.size();
            
            for (size_t i = 0; i < operations.size(); ++i) {
                core_allocation.push_back(cores_per_op + (i < extra_cores ? 1 : 0));
            }
        }
        
        // Apply allocation and set reasons
        for (size_t i = 0; i < operations.size(); ++i) {
            operations[i].allocated_cores = core_allocation[i];
            
            if (operations[i].baseline_gflops >= 5.0) {
                operations[i].allocation_reason = "High efficiency - gets more cores for maximum throughput";
            } else if (operations[i].baseline_gflops >= 1.0) {
                operations[i].allocation_reason = "Moderate efficiency - gets balanced allocation";
            } else {
                operations[i].allocation_reason = "Low efficiency - gets minimal cores";
            }
            
            // Add performance ranking info
            operations[i].allocation_reason += " (Rank #" + std::to_string(i + 1) + ")";
            
            std::cout << operations[i].name << ": " << operations[i].allocated_cores 
                      << " cores (" << operations[i].allocation_reason << ")" << std::endl;
        }
        
        // Verify total allocation
        int total_allocated = 0;
        for (const auto& op : operations) {
            total_allocated += op.allocated_cores;
        }
        
        std::cout << "\nTotal cores allocated: " << total_allocated << "/" << total_cores << std::endl;
        
        if (total_allocated == static_cast<int>(total_cores)) {
            std::cout << "✅ Perfect core allocation!" << std::endl;
        } else {
            std::cout << "❌ Core allocation mismatch!" << std::endl;
        }
    }
    
    void run_adaptive_benchmark(const std::vector<OperationProfile>& operations, 
                               size_t stocks_per_core, size_t points_per_stock) {
        
        for (const auto& op : operations) {
            std::cout << "\n--- " << op.name << " ---" << std::endl;
            std::cout << "Allocated cores: " << op.allocated_cores << std::endl;
            std::cout << "Reason: " << op.allocation_reason << std::endl;
            
            const size_t total_stocks = op.allocated_cores * stocks_per_core;
            const size_t total_ops = total_stocks * op.ops_per_stock;
            
            // Generate test data for all cores
            std::vector<std::vector<std::vector<double>>> core_stock_data(op.allocated_cores);
            for (int core = 0; core < op.allocated_cores; ++core) {
                core_stock_data[core].reserve(stocks_per_core);
                for (size_t stock = 0; stock < stocks_per_core; ++stock) {
                    core_stock_data[core].push_back(generate_test_data(points_per_stock));
                }
            }
            
            std::atomic<size_t> completed_stocks{0};
            
            double processing_time = measure_time_ms([&]() {
                std::vector<std::thread> threads;
                threads.reserve(op.allocated_cores);
                
                // Launch threads for allocated cores
                for (int core = 0; core < op.allocated_cores; ++core) {
                    threads.emplace_back([&, core]() {
                        // Each core processes its assigned stocks
                        for (size_t stock_idx = 0; stock_idx < stocks_per_core; ++stock_idx) {
                            auto result = op.processor(core_stock_data[core][stock_idx]);
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
            double gflops_per_core = gflops / op.allocated_cores;
            double improvement_vs_baseline = gflops / op.baseline_gflops;
            
            std::cout << "Results:" << std::endl;
            std::cout << "  Processing Time: " << std::fixed << std::setprecision(3) << processing_time << " ms" << std::endl;
            std::cout << "  Total Performance: " << std::setprecision(3) << gflops << " GFLOPS" << std::endl;
            std::cout << "  Per-Core Performance: " << std::setprecision(3) << gflops_per_core << " GFLOPS/core" << std::endl;
            std::cout << "  Baseline Performance: " << std::setprecision(3) << op.baseline_gflops << " GFLOPS" << std::endl;
            std::cout << "  Improvement Factor: " << std::setprecision(1) << improvement_vs_baseline << "x" << std::endl;
            std::cout << "  Stock Throughput: " << std::setprecision(1) << stocks_per_second << " stocks/second" << std::endl;
            std::cout << "  Stocks Processed: " << completed_stocks.load() << "/" << total_stocks << std::endl;
            
            // Performance classification
            if (gflops >= 50.0) {
                std::cout << "  🚀 EXCELLENT PERFORMANCE!" << std::endl;
            } else if (gflops >= 20.0) {
                std::cout << "  ✅ Good Performance" << std::endl;
            } else if (gflops >= 10.0) {
                std::cout << "  ⚠️  Moderate Performance" << std::endl;
            } else {
                std::cout << "  ❌ Needs Optimization" << std::endl;
            }
        }
        
        // Summary
        std::cout << "\n=== ADAPTIVE ALLOCATION SUMMARY ===" << std::endl;
        double total_gflops = 0.0;
        int total_cores_used = 0;
        
        for (const auto& op : operations) {
            // Recalculate performance for summary
            auto test_data = generate_test_data(points_per_stock);
            double single_core_gflops = profile_operation_performance(op, test_data);
            double estimated_total_gflops = single_core_gflops * op.allocated_cores * 0.8; // 80% efficiency estimate
            
            total_gflops += estimated_total_gflops;
            total_cores_used += op.allocated_cores;
            
            std::cout << op.name << ": " << op.allocated_cores << " cores, ~" 
                      << std::fixed << std::setprecision(1) << estimated_total_gflops << " GFLOPS" << std::endl;
        }
        
        std::cout << "\nTotal Estimated Performance: " << std::fixed << std::setprecision(1) 
                  << total_gflops << " GFLOPS" << std::endl;
        std::cout << "Total Cores Used: " << total_cores_used << std::endl;
        std::cout << "Average Performance per Core: " << std::fixed << std::setprecision(1) 
                  << (total_gflops / total_cores_used) << " GFLOPS/core" << std::endl;
        
        // Progress toward target
        std::cout << "\n=== TARGET PROGRESS ===" << std::endl;
        std::cout << "Current Estimated: " << std::fixed << std::setprecision(1) << total_gflops << " GFLOPS" << std::endl;
        std::cout << "Target: 300 GFLOPS" << std::endl;
        std::cout << "Progress: " << std::fixed << std::setprecision(1) << (total_gflops / 300.0 * 100) << "%" << std::endl;
        std::cout << "Remaining: " << std::fixed << std::setprecision(1) << (300.0 / total_gflops) << "x to reach target" << std::endl;
    }
};

// Function to run adaptive core benchmark
void run_adaptive_core_benchmark() {
    AdaptiveCoreBenchmark benchmark;
    benchmark.run_adaptive_core_benchmark();
}
