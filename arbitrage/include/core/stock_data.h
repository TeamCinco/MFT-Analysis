#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>

// Aligned allocator for SIMD operations
template<typename T, size_t Alignment>
class aligned_allocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = aligned_allocator<U, Alignment>;
    };

    aligned_allocator() = default;
    
    template<typename U>
    aligned_allocator(const aligned_allocator<U, Alignment>&) {}

    pointer allocate(size_type n) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type) {
        free(p);
    }

    bool operator==(const aligned_allocator&) const { return true; }
    bool operator!=(const aligned_allocator&) const { return false; }
};

// SIMD-optimized stock data structure
struct alignas(32) StockData {
    std::string symbol;
    std::vector<std::chrono::system_clock::time_point> timestamps;
    
    // SIMD-aligned price data for optimal performance
    std::vector<double, aligned_allocator<double, 32>> open;
    std::vector<double, aligned_allocator<double, 32>> high;
    std::vector<double, aligned_allocator<double, 32>> low;
    std::vector<double, aligned_allocator<double, 32>> close;
    std::vector<double, aligned_allocator<double, 32>> volume;
    std::vector<double, aligned_allocator<double, 32>> returns;
    
    // Pre-calculated statistics for faster analysis
    double mean_price = 0.0;
    double mean_return = 0.0;
    double volatility = 0.0;
    double min_price = 0.0;
    double max_price = 0.0;
    
    // Market classification
    std::string sector;
    std::string market_cap_bucket; // small, mid, large
    
    void reserve(size_t size) {
        timestamps.reserve(size);
        open.reserve(size);
        high.reserve(size);
        low.reserve(size);
        close.reserve(size);
        volume.reserve(size);
        returns.reserve(size);
    }
    
    size_t size() const {
        return close.size();
    }
    
    bool empty() const {
        return close.empty();
    }
    
    // Calculate basic statistics
    void calculateStatistics();
    
    // Calculate returns from prices
    void calculateReturns();
};

// Cointegration analysis result
struct CointegrationResult {
    std::string stock1;
    std::string stock2;
    double adf_statistic;           // Augmented Dickey-Fuller test statistic
    double p_value;                 // Statistical significance
    double critical_value_1pct;     // 1% critical value
    double critical_value_5pct;     // 5% critical value
    double critical_value_10pct;    // 10% critical value
    double half_life;               // Mean reversion half-life in days
    double hedge_ratio;             // Optimal hedge ratio (beta)
    double spread_mean;             // Mean of the spread
    double spread_std;              // Standard deviation of spread
    double max_spread;              // Maximum historical spread
    double min_spread;              // Minimum historical spread
    double current_spread;          // Current spread level
    double z_score;                 // Current Z-score of spread
    std::string cointegration_grade; // A, B, C, D rating
    bool is_cointegrated;           // True if statistically significant
    
    // Trading metrics
    double entry_threshold;         // Z-score threshold for entry
    double exit_threshold;          // Z-score threshold for exit
    double expected_return;         // Expected return per trade
    double sharpe_ratio;            // Historical Sharpe ratio
    int num_trades_historical;      // Number of historical trades
    double win_rate;                // Historical win rate
};

// Correlation analysis result
struct CorrelationResult {
    std::string stock1;
    std::string stock2;
    double pearson_correlation;     // Linear correlation coefficient
    double spearman_correlation;    // Rank correlation coefficient
    double kendall_tau;             // Kendall's tau correlation
    double rolling_correlation_30d; // 30-day rolling correlation
    double rolling_correlation_60d; // 60-day rolling correlation
    double correlation_stability;   // Stability measure (1 - std of rolling corr)
    double correlation_breakdown_count; // Number of times correlation broke down
    double min_correlation;         // Minimum correlation in period
    double max_correlation;         // Maximum correlation in period
    std::string correlation_grade;  // A, B, C, D rating
    
    // Sector information
    std::string sector1;
    std::string sector2;
    bool same_sector;
    
    // Price information for filtering
    double price1;
    double price2;
    bool affordable_pair;           // Both stocks < $500
};

// Combined arbitrage opportunity
struct ArbitrageOpportunity {
    std::string stock1;
    std::string stock2;
    
    // Combined scores
    double cointegration_score;     // 0-100 score
    double correlation_score;       // 0-100 score
    double combined_score;          // Weighted combination
    double profit_potential;        // Expected annual return %
    double risk_score;              // 0-100 risk assessment
    
    // Trading parameters
    double entry_z_score;
    double exit_z_score;
    double stop_loss_z_score;
    double position_size_stock1;    // Always 1 for single-share strategy
    double position_size_stock2;    // Calculated hedge ratio
    double cash_required;           // Total cash needed
    
    // Performance metrics
    double expected_sharpe_ratio;
    double max_drawdown;
    double expected_trades_per_year;
    
    // Market context
    std::string market_regime;      // bull, bear, sideways
    std::string volatility_regime;  // low, medium, high
    
    std::string opportunity_grade;  // A, B, C, D overall grade
};

// Portfolio constraints for single-share strategy
struct PortfolioConstraints {
    double max_capital = 5000.0;
    int max_positions = 5;
    double min_stock_price = 10.0;
    double max_stock_price = 500.0;
    double max_position_risk = 0.20;    // 20% max risk per position
    double min_expected_return = 0.05;   // 5% minimum expected return
    bool require_same_sector = false;
    std::vector<std::string> excluded_sectors;
};
