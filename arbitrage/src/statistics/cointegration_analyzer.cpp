#include "simd_statistics.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

// Enhanced Cointegration Analyzer Implementation
// Based on Engle-Granger two-step method

class EnhancedCointegrationAnalyzer {
public:
    // Perform comprehensive cointegration analysis
    static CointegrationResult analyzeCointegration(
        const StockData& stock1,
        const StockData& stock2,
        double significance_level = 0.05) {
        
        CointegrationResult result;
        result.stock1 = stock1.symbol;
        result.stock2 = stock2.symbol;
        
        if (stock1.close.size() != stock2.close.size() || stock1.close.size() < 50) {
            result.is_cointegrated = false;
            result.p_value = 1.0;
            return result;
        }
        
        // Step 1: Estimate hedge ratio using linear regression
        auto regression_result = estimateHedgeRatio(stock1.close, stock2.close);
        result.hedge_ratio = regression_result.second; // beta (slope)
        double alpha = regression_result.first; // intercept
        
        // Step 2: Calculate spread (residuals)
        std::vector<double> spread = calculateSpread(stock1.close, stock2.close, result.hedge_ratio);
        
        // Step 3: Perform Augmented Dickey-Fuller test on spread
        auto adf_result = augmentedDickeyFullerTest(spread);
        result.adf_statistic = adf_result.test_statistic;
        result.p_value = adf_result.p_value;
        
        // Set critical values
        result.critical_value_1pct = -3.43;
        result.critical_value_5pct = -2.86;
        result.critical_value_10pct = -2.57;
        
        // Determine if cointegrated
        result.is_cointegrated = (result.adf_statistic < result.critical_value_5pct) && 
                                (result.p_value < significance_level);
        
        // Calculate spread statistics
        calculateSpreadStatistics(spread, result);
        
        // Calculate half-life of mean reversion
        result.half_life = calculateHalfLife(spread);
        
        // Generate trading signals and metrics
        generateTradingMetrics(spread, result);
        
        // Assign grade based on statistical significance
        result.cointegration_grade = assignGrade(result);
        
        return result;
    }
    
    // Calculate correlation between two price series
    static double calculateCorrelation(
        const std::vector<double, aligned_allocator<double, 32>>& series1,
        const std::vector<double, aligned_allocator<double, 32>>& series2) {
        
        if (series1.size() != series2.size() || series1.empty()) {
            return 0.0;
        }
        
        return SIMDStatistics::calculateCorrelation_SIMD(series1, series2);
    }

private:
    // Augmented Dickey-Fuller test result
    struct ADFResult {
        double test_statistic;
        double p_value;
        int lags_used;
    };
    
    // Estimate hedge ratio using OLS regression: price2 = alpha + beta * price1 + error
    static std::pair<double, double> estimateHedgeRatio(
        const std::vector<double, aligned_allocator<double, 32>>& price1,
        const std::vector<double, aligned_allocator<double, 32>>& price2) {
        
        // Convert to standard vectors for regression
        std::vector<double> x(price1.begin(), price1.end());
        std::vector<double> y(price2.begin(), price2.end());
        
        return SIMDStatistics::linearRegression_SIMD(y, x);
    }
    
    // Calculate spread: spread = price2 - hedge_ratio * price1
    static std::vector<double> calculateSpread(
        const std::vector<double, aligned_allocator<double, 32>>& price1,
        const std::vector<double, aligned_allocator<double, 32>>& price2,
        double hedge_ratio) {
        
        std::vector<double> spread;
        spread.reserve(price1.size());
        
        for (size_t i = 0; i < price1.size(); ++i) {
            spread.push_back(price2[i] - hedge_ratio * price1[i]);
        }
        
        return spread;
    }
    
    // Augmented Dickey-Fuller test for unit root
    static ADFResult augmentedDickeyFullerTest(const std::vector<double>& series, int max_lags = 10) {
        ADFResult result;
        
        if (series.size() < 20) {
            result.test_statistic = 0.0;
            result.p_value = 1.0;
            result.lags_used = 0;
            return result;
        }
        
        // Determine optimal number of lags using AIC
        int optimal_lags = selectOptimalLags(series, max_lags);
        result.lags_used = optimal_lags;
        
        // Perform ADF test with optimal lags
        result.test_statistic = performADFTest(series, optimal_lags);
        
        // Calculate p-value using MacKinnon critical values approximation
        result.p_value = calculateADFPValue(result.test_statistic, series.size());
        
        return result;
    }
    
    // Select optimal lags using Akaike Information Criterion
    static int selectOptimalLags(const std::vector<double>& series, int max_lags) {
        double best_aic = std::numeric_limits<double>::max();
        int best_lags = 1;
        
        for (int lags = 1; lags <= std::min(max_lags, static_cast<int>(series.size() / 4)); ++lags) {
            double aic = calculateAIC(series, lags);
            if (aic < best_aic) {
                best_aic = aic;
                best_lags = lags;
            }
        }
        
        return best_lags;
    }
    
    // Calculate AIC for ADF regression with given lags
    static double calculateAIC(const std::vector<double>& series, int lags) {
        // Simplified AIC calculation for ADF regression
        // In practice, this would involve running the full ADF regression
        
        size_t n = series.size() - lags - 1;
        if (n < 10) return std::numeric_limits<double>::max();
        
        // Estimate residual sum of squares for ADF regression
        double rss = estimateADFResidualSumSquares(series, lags);
        
        // AIC = 2k + n*ln(RSS/n) where k is number of parameters
        int k = lags + 2; // intercept + trend + lagged differences
        return 2.0 * k + n * std::log(rss / n);
    }
    
    // Estimate RSS for ADF regression (simplified)
    static double estimateADFResidualSumSquares(const std::vector<double>& series, int lags) {
        // This is a simplified estimation
        // In practice, you'd run the full ADF regression
        
        std::vector<double> diff_series;
        for (size_t i = 1; i < series.size(); ++i) {
            diff_series.push_back(series[i] - series[i-1]);
        }
        
        double mean_diff = std::accumulate(diff_series.begin(), diff_series.end(), 0.0) / diff_series.size();
        
        double rss = 0.0;
        for (double diff : diff_series) {
            double residual = diff - mean_diff;
            rss += residual * residual;
        }
        
        return rss;
    }
    
    // Perform ADF test with specified lags
    static double performADFTest(const std::vector<double>& series, int lags) {
        // Simplified ADF test implementation
        // Full implementation would use matrix operations for regression
        
        if (series.size() < lags + 10) {
            return 0.0;
        }
        
        // Create lagged differences
        std::vector<double> y, x, lagged_diffs;
        
        for (size_t i = lags + 1; i < series.size(); ++i) {
            // Dependent variable: first difference
            y.push_back(series[i] - series[i-1]);
            
            // Independent variable: lagged level
            x.push_back(series[i-1]);
        }
        
        if (y.empty() || x.empty()) {
            return 0.0;
        }
        
        // Simple regression to get coefficient and standard error
        auto regression = SIMDStatistics::linearRegression_SIMD(y, x);
        double beta = regression.second;
        
        // Estimate standard error (simplified)
        double residual_sum = 0.0;
        for (size_t i = 0; i < y.size(); ++i) {
            double predicted = regression.first + beta * x[i];
            double residual = y[i] - predicted;
            residual_sum += residual * residual;
        }
        
        double mse = residual_sum / (y.size() - 2);
        double x_var = SIMDStatistics::variance_SIMD(x);
        double se_beta = std::sqrt(mse / (x_var * x.size()));
        
        // t-statistic for unit root test (H0: beta = 0)
        return beta / se_beta;
    }
    
    // Calculate p-value using MacKinnon approximation
    static double calculateADFPValue(double test_statistic, size_t sample_size) {
        // Simplified p-value calculation using critical value approximation
        // This is a rough approximation - in practice you'd use MacKinnon tables
        
        if (test_statistic < -3.43) return 0.01;   // 1% significance
        if (test_statistic < -2.86) return 0.05;   // 5% significance  
        if (test_statistic < -2.57) return 0.10;   // 10% significance
        
        // Linear interpolation for intermediate values
        if (test_statistic < -2.0) {
            return 0.10 + (test_statistic + 2.57) / (-2.0 + 2.57) * (0.30 - 0.10);
        }
        
        return std::min(0.99, 0.30 + (test_statistic + 2.0) / 2.0 * 0.69);
    }
    
    // Calculate spread statistics
    static void calculateSpreadStatistics(const std::vector<double>& spread, CointegrationResult& result) {
        if (spread.empty()) {
            result.spread_mean = 0.0;
            result.spread_std = 0.0;
            result.current_spread = 0.0;
            result.z_score = 0.0;
            return;
        }
        
        // Calculate mean and standard deviation
        result.spread_mean = std::accumulate(spread.begin(), spread.end(), 0.0) / spread.size();
        
        double variance = 0.0;
        for (double s : spread) {
            double diff = s - result.spread_mean;
            variance += diff * diff;
        }
        result.spread_std = std::sqrt(variance / spread.size());
        
        // Current spread and z-score
        result.current_spread = spread.back();
        result.z_score = (result.current_spread - result.spread_mean) / result.spread_std;
        
        // Min and max spread
        auto minmax = std::minmax_element(spread.begin(), spread.end());
        result.min_spread = *minmax.first;
        result.max_spread = *minmax.second;
    }
    
    // Calculate half-life of mean reversion using AR(1) model
    static double calculateHalfLife(const std::vector<double>& spread) {
        if (spread.size() < 10) {
            return 0.0;
        }
        
        // Create lagged series for AR(1): spread[t] = alpha + beta * spread[t-1] + error
        std::vector<double> y, x;
        for (size_t i = 1; i < spread.size(); ++i) {
            y.push_back(spread[i]);
            x.push_back(spread[i-1]);
        }
        
        auto regression = SIMDStatistics::linearRegression_SIMD(y, x);
        double beta = regression.second;
        
        // Half-life = -ln(2) / ln(beta)
        if (beta <= 0 || beta >= 1) {
            return 0.0; // No mean reversion
        }
        
        return -std::log(2.0) / std::log(beta);
    }
    
    // Generate trading metrics and signals
    static void generateTradingMetrics(const std::vector<double>& spread, CointegrationResult& result) {
        if (spread.empty()) {
            return;
        }
        
        // Set trading thresholds
        result.entry_threshold = 2.0;  // 2 standard deviations
        result.exit_threshold = 0.5;   // 0.5 standard deviations
        
        // Simulate historical trading performance
        simulateHistoricalTrades(spread, result);
    }
    
    // Simulate historical trades to calculate performance metrics
    static void simulateHistoricalTrades(const std::vector<double>& spread, CointegrationResult& result) {
        if (spread.size() < 20) {
            result.num_trades_historical = 0;
            result.win_rate = 0.0;
            result.expected_return = 0.0;
            result.sharpe_ratio = 0.0;
            return;
        }
        
        std::vector<double> z_scores;
        for (double s : spread) {
            z_scores.push_back((s - result.spread_mean) / result.spread_std);
        }
        
        std::vector<double> trade_returns;
        bool in_trade = false;
        double entry_z = 0.0;
        std::string trade_type;
        
        for (size_t i = 1; i < z_scores.size(); ++i) {
            double z = z_scores[i];
            
            if (!in_trade) {
                // Entry conditions
                if (z > result.entry_threshold) {
                    // Short spread (expect mean reversion down)
                    in_trade = true;
                    entry_z = z;
                    trade_type = "SHORT_SPREAD";
                } else if (z < -result.entry_threshold) {
                    // Long spread (expect mean reversion up)
                    in_trade = true;
                    entry_z = z;
                    trade_type = "LONG_SPREAD";
                }
            } else {
                // Exit conditions
                bool should_exit = false;
                
                if (trade_type == "SHORT_SPREAD" && z < result.exit_threshold) {
                    should_exit = true;
                } else if (trade_type == "LONG_SPREAD" && z > -result.exit_threshold) {
                    should_exit = true;
                }
                
                // Stop loss at 3 standard deviations
                if (std::abs(z) > 3.0) {
                    should_exit = true;
                }
                
                if (should_exit) {
                    // Calculate trade return (simplified)
                    double trade_return = 0.0;
                    if (trade_type == "SHORT_SPREAD") {
                        trade_return = (entry_z - z) * 0.01; // Simplified return calculation
                    } else {
                        trade_return = (z - entry_z) * 0.01;
                    }
                    
                    trade_returns.push_back(trade_return);
                    in_trade = false;
                }
            }
        }
        
        // Calculate performance metrics
        result.num_trades_historical = trade_returns.size();
        
        if (!trade_returns.empty()) {
            double total_return = std::accumulate(trade_returns.begin(), trade_returns.end(), 0.0);
            result.expected_return = total_return / trade_returns.size();
            
            int winning_trades = std::count_if(trade_returns.begin(), trade_returns.end(), 
                                             [](double r) { return r > 0; });
            result.win_rate = static_cast<double>(winning_trades) / trade_returns.size();
            
            // Calculate Sharpe ratio (simplified)
            if (trade_returns.size() > 1) {
                double mean_return = result.expected_return;
                double variance = 0.0;
                for (double r : trade_returns) {
                    variance += (r - mean_return) * (r - mean_return);
                }
                double std_return = std::sqrt(variance / (trade_returns.size() - 1));
                result.sharpe_ratio = (std_return > 0) ? mean_return / std_return : 0.0;
            }
        }
    }
    
    // Assign grade based on statistical significance and trading metrics
    static std::string assignGrade(const CointegrationResult& result) {
        if (!result.is_cointegrated) {
            return "D";
        }
        
        double score = 0.0;
        
        // Statistical significance (40% weight)
        if (result.p_value < 0.01) score += 40;
        else if (result.p_value < 0.05) score += 30;
        else if (result.p_value < 0.10) score += 20;
        
        // Half-life (30% weight) - prefer 5-50 days
        if (result.half_life >= 5 && result.half_life <= 50) {
            score += 30;
        } else if (result.half_life > 0 && result.half_life < 100) {
            score += 15;
        }
        
        // Win rate (20% weight)
        if (result.win_rate > 0.6) score += 20;
        else if (result.win_rate > 0.5) score += 15;
        else if (result.win_rate > 0.4) score += 10;
        
        // Expected return (10% weight)
        if (result.expected_return > 0.02) score += 10;
        else if (result.expected_return > 0.01) score += 5;
        
        // Assign letter grade
        if (score >= 80) return "A";
        if (score >= 65) return "B";
        if (score >= 50) return "C";
        return "D";
    }
};

// Update the existing SIMD cointegration analyzer to use the enhanced version
CointegrationResult SIMDCointegrationAnalyzer::analyzeCointegration_SIMD(
    const StockData& stock1,
    const StockData& stock2) {
    
    return EnhancedCointegrationAnalyzer::analyzeCointegration(stock1, stock2);
}

// Batch analysis using enhanced analyzer
std::vector<CointegrationResult> SIMDCointegrationAnalyzer::batchAnalyzeCointegration_SIMD(
    const std::vector<std::pair<const StockData*, const StockData*>>& stock_pairs) {
    
    std::vector<CointegrationResult> results;
    results.reserve(stock_pairs.size());
    
    for (const auto& pair : stock_pairs) {
        if (pair.first && pair.second) {
            results.push_back(EnhancedCointegrationAnalyzer::analyzeCointegration(*pair.first, *pair.second));
        }
    }
    
    return results;
}
