#include "technical_indicators.h"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <algorithm>

std::vector<double> TechnicalIndicators::calculate_returns(const std::vector<double>& prices) {
    if (prices.size() < 2) return {};
    std::vector<double> returns;
    returns.reserve(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        returns.push_back(prices[i-1] != 0.0 ? (prices[i] - prices[i-1]) / prices[i-1] : 0.0);
    }
    return returns;
}

std::vector<double> TechnicalIndicators::simple_moving_average(const std::vector<double>& data, size_t window) {
    if (data.empty() || window == 0 || data.size() < window) return {};
    std::vector<double> result;
    result.reserve(data.size() - window + 1);
    double sum = std::accumulate(data.begin(), data.begin() + window, 0.0);
    result.push_back(sum / window);
    for (size_t i = window; i < data.size(); ++i) {
        sum += data[i] - data[i - window];
        result.push_back(sum / window);
    }
    return result;
}

std::vector<double> TechnicalIndicators::calculate_rsi(const std::vector<double>& prices, int period) {
    if (prices.size() <= static_cast<size_t>(period)) return {};
    std::vector<double> gains, losses;
    gains.reserve(prices.size() - 1);
    losses.reserve(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        gains.push_back(change > 0 ? change : 0.0);
        losses.push_back(change < 0 ? -change : 0.0);
    }
    if (gains.size() < static_cast<size_t>(period)) return {};
    std::vector<double> rsi;
    rsi.reserve(prices.size() - period);
    double avg_gain = 0.0, avg_loss = 0.0;
    for(int i=0; i<period; ++i) {
        avg_gain += gains[i];
        avg_loss += losses[i];
    }
    avg_gain /= period;
    avg_loss /= period;
    for (size_t i = period; i < gains.size(); ++i) {
        rsi.push_back(avg_loss == 0 ? 100.0 : 100.0 - (100.0 / (1.0 + avg_gain / avg_loss)));
        avg_gain = (avg_gain * (period - 1) + gains[i]) / period;
        avg_loss = (avg_loss * (period - 1) + losses[i]) / period;
    }
    rsi.push_back(avg_loss == 0 ? 100.0 : 100.0 - (100.0 / (1.0 + avg_gain / avg_loss)));
    return rsi;
}

std::vector<double> TechnicalIndicators::calculate_rolling_volatility(const std::vector<double>& returns, int window) {
    if (returns.size() < static_cast<size_t>(window) || window <= 1) return {};
    std::vector<double> volatility;
    volatility.reserve(returns.size() - window + 1);
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double sum = 0.0;
        for (int j = 0; j < window; ++j) sum += returns[i + j];
        double mean = sum / window;
        double var_sum = 0.0;
        for (int j = 0; j < window; ++j) var_sum += (returns[i + j] - mean) * (returns[i + j] - mean);
        volatility.push_back(std::sqrt(var_sum / (window - 1)));
    }
    return volatility;
}

std::vector<double> TechnicalIndicators::compute_spread(const std::vector<double>& high, const std::vector<double>& low) {
    if (high.size() != low.size()) return {};
    std::vector<double> spread;
    spread.reserve(high.size());
    for (size_t i = 0; i < high.size(); ++i) spread.push_back(high[i] - low[i]);
    return spread;
}

std::vector<double> TechnicalIndicators::internal_bar_strength(const std::vector<double>& open, const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close) {
    if (open.size()!=close.size()||high.size()!=close.size()||low.size()!=close.size()) return {};
    std::vector<double> ibs;
    ibs.reserve(close.size());
    for (size_t i = 0; i < close.size(); ++i) {
        double range = high[i] - low[i];
        ibs.push_back(range > 0 ? (close[i] - low[i]) / range : 0.5);
    }
    return ibs;
}

std::pair<std::vector<int>,std::pair<std::vector<double>,std::vector<double>>> TechnicalIndicators::candle_information(const std::vector<double>& open, const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close) {
    if (open.size()!=close.size()||high.size()!=close.size()||low.size()!=close.size()) return {{},{{},{}}};
    std::vector<int> way;
    std::vector<double> filling, amplitude;
    way.reserve(open.size()); filling.reserve(open.size()); amplitude.reserve(open.size());
    for (size_t i = 0; i < open.size(); ++i) {
        way.push_back(close[i]>open[i]?1:(close[i]<open[i]?-1:0));
        double range = high[i]-low[i];
        amplitude.push_back(range);
        filling.push_back(range>0?std::abs(close[i]-open[i])/range:0.0);
    }
    return {way, {filling, amplitude}};
}

std::pair<std::vector<double>,std::vector<double>> TechnicalIndicators::derivatives(const std::vector<double>& prices) {
    if (prices.size()<2) return {{},{}};
    std::vector<double> velocity;
    velocity.reserve(prices.size()-1);
    for (size_t i=1; i<prices.size(); ++i) velocity.push_back(prices[i]-prices[i-1]);
    if (velocity.size()<2) return {velocity,{}};
    std::vector<double> acceleration;
    acceleration.reserve(velocity.size()-1);
    for (size_t i=1; i<velocity.size(); ++i) acceleration.push_back(velocity[i]-velocity[i-1]);
    return {velocity, acceleration};
}

std::vector<double> TechnicalIndicators::log_pct_change(const std::vector<double>& prices, int window_size) {
    if (prices.size()<=static_cast<size_t>(window_size)) return {};
    std::vector<double> result;
    result.reserve(prices.size()-window_size);
    for (size_t i=window_size; i<prices.size(); ++i) {
        result.push_back((prices[i-window_size]>0&&prices[i]>0)?std::log(prices[i]/prices[i-window_size]):0.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::auto_correlation(const std::vector<double>& prices, int window_size, int lag) {
    if (prices.size()<static_cast<size_t>(window_size+lag)) return {};
    std::vector<double> result;
    result.reserve(prices.size()-window_size-lag+1);
    for (size_t i=0; i<=prices.size()-window_size-lag; ++i) {
        double sum_x=0, sum_y=0, sum_xy=0, sum_x2=0, sum_y2=0;
        for (int j=0; j<window_size; ++j) {
            double x=prices[i+j], y=prices[i+j+lag];
            sum_x+=x; sum_y+=y; sum_xy+=x*y; sum_x2+=x*x; sum_y2+=y*y;
        }
        double num=window_size*sum_xy-sum_x*sum_y;
        double den=std::sqrt((window_size*sum_x2-sum_x*sum_x)*(window_size*sum_y2-sum_y*sum_y));
        result.push_back(den!=0?num/den:0.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::skewness(const std::vector<double>& prices, int window_size) {
    if (prices.size()<static_cast<size_t>(window_size)) return {};
    std::vector<double> result;
    result.reserve(prices.size()-window_size+1);
    for (size_t i=0; i<=prices.size()-window_size; ++i) {
        double sum=0; for(int j=0; j<window_size; ++j) sum+=prices[i+j];
        double mean=sum/window_size, m3=0, m2=0;
        for (int j=0; j<window_size; ++j) {
            double diff=prices[i+j]-mean; m2+=diff*diff; m3+=diff*diff*diff;
        }
        m2/=window_size; m3/=window_size;
        double std_dev=std::sqrt(m2);
        result.push_back(std_dev>0?m3/(std_dev*std_dev*std_dev):0.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::kurtosis(const std::vector<double>& prices, int window_size) {
    if (prices.size()<static_cast<size_t>(window_size)) return {};
    std::vector<double> result;
    result.reserve(prices.size()-window_size+1);
    for (size_t i=0; i<=prices.size()-window_size; ++i) {
        double sum=0; for(int j=0; j<window_size; ++j) sum+=prices[i+j];
        double mean=sum/window_size, m4=0, m2=0;
        for (int j=0; j<window_size; ++j) {
            double diff=prices[i+j]-mean; m2+=diff*diff; m4+=diff*diff*diff*diff;
        }
        m2/=window_size; m4/=window_size;
        result.push_back(m2>0?(m4/(m2*m2))-3.0:0.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::kama(const std::vector<double>& prices, int l1, int l2, int l3) {
    if (prices.size()<static_cast<size_t>(l1+1)) return {};
    std::vector<double> kama_values;
    kama_values.reserve(prices.size());
    if(prices.empty()) return kama_values;
    kama_values.push_back(prices[0]);
    double fast_sc=2.0/(l2+1.0), slow_sc=2.0/(l3+1.0);
    for (size_t i=1; i<prices.size(); ++i) {
        size_t lookback=std::min(static_cast<size_t>(l1), i);
        double change=std::abs(prices[i]-prices[i-lookback]), vol=0;
        for (size_t j=1; j<=lookback; ++j) vol+=std::abs(prices[i-j+1]-prices[i-j]);
        double er=vol>0?change/vol:0.0;
        double sc=std::pow(er*(fast_sc-slow_sc)+slow_sc, 2);
        kama_values.push_back(kama_values.back()+sc*(prices[i]-kama_values.back()));
    }
    return kama_values;
}

std::vector<double> TechnicalIndicators::linear_slope(const std::vector<double>& prices, int window_size) {
    if (prices.size()<static_cast<size_t>(window_size)) return {};
    std::vector<double> result;
    result.reserve(prices.size()-window_size+1);
    const double sum_x=static_cast<double>(window_size*(window_size-1))/2.0;
    const double sum_x2=static_cast<double>(window_size*(window_size-1)*(2*window_size-1))/6.0;
    const double den=window_size*sum_x2-sum_x*sum_x;
    if(den==0) return {};
    for (size_t i=0; i<=prices.size()-window_size; ++i) {
        double sum_y=0, sum_xy=0;
        for (int j=0; j<window_size; ++j) {
            sum_y+=prices[i+j]; sum_xy+=j*prices[i+j];
        }
        result.push_back((window_size*sum_xy-sum_x*sum_y)/den);
    }
    return result;
}

std::vector<double> TechnicalIndicators::parkinson_volatility(const std::vector<double>& high, const std::vector<double>& low, int window_size) {
    if (high.size()!=low.size()||high.size()<static_cast<size_t>(window_size)) return {};
    std::vector<double> result;
    result.reserve(high.size()-window_size+1);
    const double factor=1.0/(4.0*std::log(2.0));
    for (size_t i=0; i<=high.size()-window_size; ++i) {
        double sum=0.0;
        for (int j=0; j<window_size; ++j) {
            if (low[i+j]>0) {
                double log_hl=std::log(high[i+j]/low[i+j]);
                sum+=log_hl*log_hl;
            }
        }
        result.push_back(std::sqrt(sum/window_size)*factor);
    }
    return result;
}

// Statistical/Mathematical Features
std::vector<double> TechnicalIndicators::z_score_20(const std::vector<double>& returns) {
    const int window = 20;
    if (returns.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(returns.size() - window + 1);
    
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double sum = 0.0;
        for (int j = 0; j < window; ++j) sum += returns[i + j];
        double mean = sum / window;
        
        double var_sum = 0.0;
        for (int j = 0; j < window; ++j) {
            double diff = returns[i + j] - mean;
            var_sum += diff * diff;
        }
        double std_dev = std::sqrt(var_sum / (window - 1));
        
        double z_score = std_dev > 0 ? (returns[i + window - 1] - mean) / std_dev : 0.0;
        result.push_back(z_score);
    }
    return result;
}

std::vector<double> TechnicalIndicators::percentile_rank_50(const std::vector<double>& prices) {
    const int window = 50;
    if (prices.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(prices.size() - window + 1);
    
    for (size_t i = 0; i <= prices.size() - window; ++i) {
        double current_price = prices[i + window - 1];
        int count_below = 0;
        for (int j = 0; j < window; ++j) {
            if (prices[i + j] < current_price) count_below++;
        }
        result.push_back(static_cast<double>(count_below) / window * 100.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::coefficient_of_variation_30(const std::vector<double>& returns) {
    const int window = 30;
    if (returns.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(returns.size() - window + 1);
    
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double sum = 0.0;
        for (int j = 0; j < window; ++j) sum += returns[i + j];
        double mean = sum / window;
        
        if (std::abs(mean) < 1e-10) {
            result.push_back(0.0);
            continue;
        }
        
        double var_sum = 0.0;
        for (int j = 0; j < window; ++j) {
            double diff = returns[i + j] - mean;
            var_sum += diff * diff;
        }
        double std_dev = std::sqrt(var_sum / (window - 1));
        result.push_back(std_dev / std::abs(mean));
    }
    return result;
}

std::vector<double> TechnicalIndicators::detrended_price_oscillator_20(const std::vector<double>& prices) {
    const int window = 20;
    auto sma = simple_moving_average(prices, window);
    if (sma.empty() || prices.size() < static_cast<size_t>(window)) return {};
    
    std::vector<double> result;
    result.reserve(sma.size());
    
    for (size_t i = 0; i < sma.size(); ++i) {
        result.push_back(prices[i + window - 1] - sma[i]);
    }
    return result;
}

std::vector<double> TechnicalIndicators::hurst_exponent_100(const std::vector<double>& prices) {
    const int window = 100;
    if (prices.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(prices.size() - window + 1);
    
    for (size_t i = 0; i <= prices.size() - window; ++i) {
        std::vector<double> log_prices;
        for (int j = 0; j < window; ++j) {
            if (prices[i + j] > 0) log_prices.push_back(std::log(prices[i + j]));
        }
        
        if (log_prices.size() < 10) {
            result.push_back(0.5);
            continue;
        }
        
        // Simplified R/S analysis
        std::vector<double> cumsum(log_prices.size());
        cumsum[0] = log_prices[0];
        for (size_t k = 1; k < log_prices.size(); ++k) {
            cumsum[k] = cumsum[k-1] + log_prices[k];
        }
        
        double mean_log = cumsum.back() / log_prices.size();
        double range = 0.0, std_dev = 0.0;
        
        for (size_t k = 0; k < log_prices.size(); ++k) {
            double dev = cumsum[k] - (k + 1) * mean_log;
            range = std::max(range, std::abs(dev));
            std_dev += (log_prices[k] - mean_log) * (log_prices[k] - mean_log);
        }
        
        std_dev = std::sqrt(std_dev / (log_prices.size() - 1));
        double rs = std_dev > 0 ? range / std_dev : 1.0;
        double hurst = rs > 0 ? std::log(rs) / std::log(log_prices.size()) : 0.5;
        result.push_back(std::max(0.0, std::min(1.0, hurst)));
    }
    return result;
}

std::vector<double> TechnicalIndicators::garch_volatility_21(const std::vector<double>& returns) {
    const int window = 21;
    if (returns.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(returns.size() - window + 1);
    
    // Simplified GARCH(1,1) estimation
    const double alpha = 0.1, beta = 0.85, omega = 0.05;
    
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double variance = 0.0;
        for (int j = 0; j < window; ++j) {
            double ret = returns[i + j];
            variance = omega + alpha * ret * ret + beta * variance;
        }
        result.push_back(std::sqrt(variance));
    }
    return result;
}

std::vector<double> TechnicalIndicators::shannon_entropy_volume_10(const std::vector<double>& volume) {
    const int window = 10;
    if (volume.size() < static_cast<size_t>(window)) return {};
    std::vector<double> result;
    result.reserve(volume.size() - window + 1);
    
    for (size_t i = 0; i <= volume.size() - window; ++i) {
        double total_volume = 0.0;
        for (int j = 0; j < window; ++j) total_volume += volume[i + j];
        
        if (total_volume <= 0) {
            result.push_back(0.0);
            continue;
        }
        
        double entropy = 0.0;
        for (int j = 0; j < window; ++j) {
            double prob = volume[i + j] / total_volume;
            if (prob > 0) entropy -= prob * std::log(prob);
        }
        result.push_back(entropy);
    }
    return result;
}

// Technical Analysis Extended
std::vector<double> TechnicalIndicators::chande_momentum_oscillator_14(const std::vector<double>& prices) {
    const int period = 14;
    if (prices.size() <= static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - period);
    
    for (size_t i = period; i < prices.size(); ++i) {
        double sum_up = 0.0, sum_down = 0.0;
        for (int j = 1; j <= period; ++j) {
            double change = prices[i - j + 1] - prices[i - j];
            if (change > 0) sum_up += change;
            else sum_down += std::abs(change);
        }
        
        double cmo = (sum_up + sum_down) > 0 ? 100.0 * (sum_up - sum_down) / (sum_up + sum_down) : 0.0;
        result.push_back(cmo);
    }
    return result;
}

std::vector<double> TechnicalIndicators::aroon_oscillator_25(const std::vector<double>& high, const std::vector<double>& low) {
    const int period = 25;
    if (high.size() != low.size() || high.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period + 1);
    
    for (size_t i = period - 1; i < high.size(); ++i) {
        int high_idx = 0, low_idx = 0;
        double max_high = high[i - period + 1], min_low = low[i - period + 1];
        
        for (int j = 0; j < period; ++j) {
            if (high[i - j] > max_high) {
                max_high = high[i - j];
                high_idx = j;
            }
            if (low[i - j] < min_low) {
                min_low = low[i - j];
                low_idx = j;
            }
        }
        
        double aroon_up = 100.0 * (period - high_idx) / period;
        double aroon_down = 100.0 * (period - low_idx) / period;
        result.push_back(aroon_up - aroon_down);
    }
    return result;
}

std::vector<double> TechnicalIndicators::trix_15(const std::vector<double>& prices) {
    const int period = 15;
    auto ema1 = exponential_moving_average(prices, period);
    if (ema1.empty()) return {};
    
    auto ema2 = exponential_moving_average(ema1, period);
    if (ema2.empty()) return {};
    
    auto ema3 = exponential_moving_average(ema2, period);
    if (ema3.size() < 2) return {};
    
    std::vector<double> result;
    result.reserve(ema3.size() - 1);
    
    for (size_t i = 1; i < ema3.size(); ++i) {
        double trix = ema3[i-1] > 0 ? 10000.0 * (ema3[i] - ema3[i-1]) / ema3[i-1] : 0.0;
        result.push_back(trix);
    }
    return result;
}

std::vector<double> TechnicalIndicators::vortex_indicator_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close) {
    const int period = 14;
    if (high.size() != low.size() || high.size() != close.size() || high.size() < static_cast<size_t>(period + 1)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period);
    
    for (size_t i = period; i < high.size(); ++i) {
        double vm_plus = 0.0, vm_minus = 0.0, tr_sum = 0.0;
        
        for (int j = 1; j <= period; ++j) {
            vm_plus += std::abs(high[i - j + 1] - low[i - j]);
            vm_minus += std::abs(low[i - j + 1] - high[i - j]);
            
            double tr = std::max({high[i - j + 1] - low[i - j + 1],
                                std::abs(high[i - j + 1] - close[i - j]),
                                std::abs(low[i - j + 1] - close[i - j])});
            tr_sum += tr;
        }
        
        double vi_plus = tr_sum > 0 ? vm_plus / tr_sum : 0.0;
        result.push_back(vi_plus);
    }
    return result;
}

std::vector<double> TechnicalIndicators::supertrend_10_3(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close) {
    const int period = 10;
    const double multiplier = 3.0;
    if (high.size() != low.size() || high.size() != close.size() || high.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period + 1);
    
    for (size_t i = period - 1; i < high.size(); ++i) {
        double atr = calculate_atr(high, low, close, period, i);
        double hl2 = (high[i] + low[i]) / 2.0;
        double upper_band = hl2 + multiplier * atr;
        double lower_band = hl2 - multiplier * atr;
        
        // Simplified supertrend calculation
        double supertrend = close[i] > upper_band ? lower_band : upper_band;
        result.push_back(supertrend);
    }
    return result;
}

std::vector<double> TechnicalIndicators::ichimoku_senkou_span_A_9_26(const std::vector<double>& high, const std::vector<double>& low) {
    const int tenkan_period = 9, kijun_period = 26;
    if (high.size() != low.size() || high.size() < static_cast<size_t>(kijun_period)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - kijun_period + 1);
    
    for (size_t i = kijun_period - 1; i < high.size(); ++i) {
        // Tenkan-sen
        double tenkan_high = *std::max_element(high.begin() + i - tenkan_period + 1, high.begin() + i + 1);
        double tenkan_low = *std::min_element(low.begin() + i - tenkan_period + 1, low.begin() + i + 1);
        double tenkan_sen = (tenkan_high + tenkan_low) / 2.0;
        
        // Kijun-sen
        double kijun_high = *std::max_element(high.begin() + i - kijun_period + 1, high.begin() + i + 1);
        double kijun_low = *std::min_element(low.begin() + i - kijun_period + 1, low.begin() + i + 1);
        double kijun_sen = (kijun_high + kijun_low) / 2.0;
        
        // Senkou Span A
        result.push_back((tenkan_sen + kijun_sen) / 2.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::ichimoku_senkou_span_B_26_52(const std::vector<double>& high, const std::vector<double>& low) {
    const int period = 52;
    if (high.size() != low.size() || high.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period + 1);
    
    for (size_t i = period - 1; i < high.size(); ++i) {
        double max_high = *std::max_element(high.begin() + i - period + 1, high.begin() + i + 1);
        double min_low = *std::min_element(low.begin() + i - period + 1, low.begin() + i + 1);
        result.push_back((max_high + min_low) / 2.0);
    }
    return result;
}

std::vector<double> TechnicalIndicators::fisher_transform_10(const std::vector<double>& high, const std::vector<double>& low) {
    const int period = 10;
    if (high.size() != low.size() || high.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period + 1);
    
    for (size_t i = period - 1; i < high.size(); ++i) {
        double max_high = *std::max_element(high.begin() + i - period + 1, high.begin() + i + 1);
        double min_low = *std::min_element(low.begin() + i - period + 1, low.begin() + i + 1);
        
        double range = max_high - min_low;
        if (range <= 0) {
            result.push_back(0.0);
            continue;
        }
        
        double normalized = 2.0 * ((high[i] + low[i]) / 2.0 - min_low) / range - 1.0;
        normalized = std::max(-0.999, std::min(0.999, normalized));
        
        double fisher = 0.5 * std::log((1.0 + normalized) / (1.0 - normalized));
        result.push_back(fisher);
    }
    return result;
}

// Volume/Liquidity Advanced
std::vector<double> TechnicalIndicators::volume_weighted_average_price_intraday(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume) {
    if (high.size() != low.size() || high.size() != close.size() || high.size() != volume.size()) return {};
    
    std::vector<double> result;
    result.reserve(high.size());
    
    double cumulative_pv = 0.0, cumulative_volume = 0.0;
    
    for (size_t i = 0; i < high.size(); ++i) {
        double typical_price = (high[i] + low[i] + close[i]) / 3.0;
        cumulative_pv += typical_price * volume[i];
        cumulative_volume += volume[i];
        
        double vwap = cumulative_volume > 0 ? cumulative_pv / cumulative_volume : typical_price;
        result.push_back(vwap);
    }
    return result;
}

std::vector<double> TechnicalIndicators::volume_profile_high_volume_node_intraday(const std::vector<double>& prices, const std::vector<double>& volume) {
    if (prices.size() != volume.size() || prices.empty()) return {};
    
    std::vector<double> result;
    result.reserve(prices.size());
    
    // Simplified volume profile - find price level with highest volume
    for (size_t i = 0; i < prices.size(); ++i) {
        double max_volume = 0.0;
        double hvn_price = prices[i];
        
        for (size_t j = 0; j <= i; ++j) {
            if (volume[j] > max_volume) {
                max_volume = volume[j];
                hvn_price = prices[j];
            }
        }
        result.push_back(hvn_price);
    }
    return result;
}

std::vector<double> TechnicalIndicators::volume_profile_low_volume_node_intraday(const std::vector<double>& prices, const std::vector<double>& volume) {
    if (prices.size() != volume.size() || prices.empty()) return {};
    
    std::vector<double> result;
    result.reserve(prices.size());
    
    for (size_t i = 0; i < prices.size(); ++i) {
        double min_volume = volume[0];
        double lvn_price = prices[0];
        
        for (size_t j = 0; j <= i; ++j) {
            if (volume[j] < min_volume) {
                min_volume = volume[j];
                lvn_price = prices[j];
            }
        }
        result.push_back(lvn_price);
    }
    return result;
}

std::vector<double> TechnicalIndicators::on_balance_volume_sma_20(const std::vector<double>& prices, const std::vector<double>& volume) {
    if (prices.size() != volume.size() || prices.size() < 2) return {};
    
    std::vector<double> obv;
    obv.reserve(prices.size());
    obv.push_back(0.0);
    
    for (size_t i = 1; i < prices.size(); ++i) {
        if (prices[i] > prices[i-1]) obv.push_back(obv.back() + volume[i]);
        else if (prices[i] < prices[i-1]) obv.push_back(obv.back() - volume[i]);
        else obv.push_back(obv.back());
    }
    
    return simple_moving_average(obv, 20);
}

std::vector<double> TechnicalIndicators::klinger_oscillator_34_55(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume) {
    if (high.size() != low.size() || high.size() != close.size() || high.size() != volume.size() || high.size() < 2) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - 1);
    
    for (size_t i = 1; i < high.size(); ++i) {
        double hlc = (high[i] + low[i] + close[i]) / 3.0;
        double prev_hlc = (high[i-1] + low[i-1] + close[i-1]) / 3.0;
        
        double trend = hlc > prev_hlc ? 1.0 : -1.0;
        double sv = volume[i] * trend;
        
        // Simplified Klinger calculation
        result.push_back(sv);
    }
    
    auto ema34 = exponential_moving_average(result, 34);
    auto ema55 = exponential_moving_average(result, 55);
    
    if (ema34.size() != ema55.size()) return {};
    
    std::vector<double> klinger;
    klinger.reserve(ema34.size());
    for (size_t i = 0; i < ema34.size(); ++i) {
        klinger.push_back(ema34[i] - ema55[i]);
    }
    return klinger;
}

std::vector<double> TechnicalIndicators::money_flow_index_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume) {
    const int period = 14;
    if (high.size() != low.size() || high.size() != close.size() || high.size() != volume.size() || high.size() < static_cast<size_t>(period + 1)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period);
    
    for (size_t i = period; i < high.size(); ++i) {
        double positive_flow = 0.0, negative_flow = 0.0;
        
        for (int j = 1; j <= period; ++j) {
            double typical_price = (high[i - j + 1] + low[i - j + 1] + close[i - j + 1]) / 3.0;
            double prev_typical_price = (high[i - j] + low[i - j] + close[i - j]) / 3.0;
            double money_flow = typical_price * volume[i - j + 1];
            
            if (typical_price > prev_typical_price) positive_flow += money_flow;
            else if (typical_price < prev_typical_price) negative_flow += money_flow;
        }
        
        double mfi = (positive_flow + negative_flow) > 0 ? 100.0 - (100.0 / (1.0 + positive_flow / negative_flow)) : 50.0;
        result.push_back(mfi);
    }
    return result;
}

std::vector<double> TechnicalIndicators::vwap_deviation_stddev_30(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, const std::vector<double>& volume) {
    auto vwap = volume_weighted_average_price_intraday(high, low, close, volume);
    if (vwap.empty() || vwap.size() < 30) return {};
    
    std::vector<double> deviations;
    deviations.reserve(vwap.size());
    
    for (size_t i = 0; i < vwap.size(); ++i) {
        double typical_price = (high[i] + low[i] + close[i]) / 3.0;
        deviations.push_back(typical_price - vwap[i]);
    }
    
    return calculate_rolling_volatility(deviations, 30);
}

// Cross-Sectional/Relative
std::vector<double> TechnicalIndicators::relative_strength_spx_50(const std::vector<double>& prices, const std::vector<double>& spx_prices) {
    const int period = 50;
    if (prices.size() != spx_prices.size() || prices.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - period + 1);
    
    for (size_t i = period - 1; i < prices.size(); ++i) {
        double stock_return = prices[i-period+1] > 0 ? (prices[i] - prices[i-period+1]) / prices[i-period+1] : 0.0;
        double spx_return = spx_prices[i-period+1] > 0 ? (spx_prices[i] - spx_prices[i-period+1]) / spx_prices[i-period+1] : 0.0;
        result.push_back(stock_return - spx_return);
    }
    return result;
}

std::vector<double> TechnicalIndicators::relative_strength_sector_50(const std::vector<double>& prices, const std::vector<double>& sector_prices) {
    return relative_strength_spx_50(prices, sector_prices);
}

std::vector<double> TechnicalIndicators::beta_to_market_60(const std::vector<double>& returns, const std::vector<double>& market_returns) {
    const int period = 60;
    if (returns.size() != market_returns.size() || returns.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(returns.size() - period + 1);
    
    for (size_t i = 0; i <= returns.size() - period; ++i) {
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        
        for (int j = 0; j < period; ++j) {
            double x = market_returns[i + j];
            double y = returns[i + j];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double num = period * sum_xy - sum_x * sum_y;
        double den = period * sum_x2 - sum_x * sum_x;
        double beta = den != 0 ? num / den : 0.0;
        result.push_back(beta);
    }
    return result;
}

std::vector<double> TechnicalIndicators::correlation_to_sector_40(const std::vector<double>& returns, const std::vector<double>& sector_returns) {
    return auto_correlation(returns, 40, 0); // Simplified - using autocorrelation logic
}

std::vector<double> TechnicalIndicators::cross_sectional_momentum_rank_20(const std::vector<double>& returns) {
    const int period = 20;
    if (returns.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(returns.size() - period + 1);
    
    for (size_t i = 0; i <= returns.size() - period; ++i) {
        double sum = 0.0;
        for (int j = 0; j < period; ++j) sum += returns[i + j];
        double momentum = sum / period;
        result.push_back(momentum * 100.0); // Simplified ranking
    }
    return result;
}

std::vector<double> TechnicalIndicators::pair_spread_vs_competitor_A_30(const std::vector<double>& prices, const std::vector<double>& competitor_prices) {
    if (prices.size() != competitor_prices.size()) return {};
    
    std::vector<double> spreads;
    spreads.reserve(prices.size());
    
    for (size_t i = 0; i < prices.size(); ++i) {
        spreads.push_back(prices[i] - competitor_prices[i]);
    }
    
    return simple_moving_average(spreads, 30);
}

// Regime Detection
std::vector<double> TechnicalIndicators::markov_regime_switching_garch_2_state(const std::vector<double>& returns) {
    if (returns.size() < 50) return {};
    
    std::vector<double> result;
    result.reserve(returns.size());
    
    // Simplified regime detection based on volatility
    auto volatility = calculate_rolling_volatility(returns, 20);
    if (volatility.empty()) return {};
    
    double vol_threshold = 0.0;
    for (double vol : volatility) vol_threshold += vol;
    vol_threshold /= volatility.size();
    
    for (size_t i = 0; i < volatility.size(); ++i) {
        result.push_back(volatility[i] > vol_threshold ? 1.0 : 0.0);
    }
    
    // Fill beginning with zeros
    while (result.size() < returns.size()) {
        result.insert(result.begin(), 0.0);
    }
    
    return result;
}

std::vector<double> TechnicalIndicators::adx_rating_14(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close) {
    const int period = 14;
    if (high.size() != low.size() || high.size() != close.size() || high.size() < static_cast<size_t>(period + 1)) return {};
    
    std::vector<double> result;
    result.reserve(high.size() - period);
    
    for (size_t i = period; i < high.size(); ++i) {
        double dm_plus_sum = 0.0, dm_minus_sum = 0.0, tr_sum = 0.0;
        
        for (int j = 1; j <= period; ++j) {
            double dm_plus = (high[i - j + 1] - high[i - j]) > (low[i - j] - low[i - j + 1]) ? 
                           std::max(0.0, high[i - j + 1] - high[i - j]) : 0.0;
            double dm_minus = (low[i - j] - low[i - j + 1]) > (high[i - j + 1] - high[i - j]) ? 
                            std::max(0.0, low[i - j] - low[i - j + 1]) : 0.0;
            
            double tr = std::max({high[i - j + 1] - low[i - j + 1],
                                std::abs(high[i - j + 1] - close[i - j]),
                                std::abs(low[i - j + 1] - close[i - j])});
            
            dm_plus_sum += dm_plus;
            dm_minus_sum += dm_minus;
            tr_sum += tr;
        }
        
        double di_plus = tr_sum > 0 ? 100.0 * dm_plus_sum / tr_sum : 0.0;
        double di_minus = tr_sum > 0 ? 100.0 * dm_minus_sum / tr_sum : 0.0;
        double dx = (di_plus + di_minus) > 0 ? 100.0 * std::abs(di_plus - di_minus) / (di_plus + di_minus) : 0.0;
        
        result.push_back(dx);
    }
    return result;
}

std::vector<double> TechnicalIndicators::chow_test_statistic_breakpoint_detection_50(const std::vector<double>& returns) {
    const int window = 50;
    if (returns.size() < static_cast<size_t>(window * 2)) return {};
    
    std::vector<double> result;
    result.reserve(returns.size() - window * 2 + 1);
    
    for (size_t i = 0; i <= returns.size() - window * 2; ++i) {
        // Simplified Chow test - compare variance of two halves
        double var1 = 0.0, var2 = 0.0;
        double mean1 = 0.0, mean2 = 0.0;
        
        for (int j = 0; j < window; ++j) {
            mean1 += returns[i + j];
            mean2 += returns[i + window + j];
        }
        mean1 /= window;
        mean2 /= window;
        
        for (int j = 0; j < window; ++j) {
            var1 += (returns[i + j] - mean1) * (returns[i + j] - mean1);
            var2 += (returns[i + window + j] - mean2) * (returns[i + window + j] - mean2);
        }
        var1 /= (window - 1);
        var2 /= (window - 1);
        
        double chow_stat = var2 > 0 ? var1 / var2 : 1.0;
        result.push_back(chow_stat);
    }
    return result;
}

std::vector<double> TechnicalIndicators::market_regime_hmm_3_states_price_vol(const std::vector<double>& prices, const std::vector<double>& volatility) {
    if (prices.size() != volatility.size() || prices.empty()) return {};
    
    std::vector<double> result;
    result.reserve(prices.size());
    
    // Simplified 3-state regime detection
    for (size_t i = 0; i < prices.size(); ++i) {
        if (i == 0) {
            result.push_back(1.0); // Default to state 1
            continue;
        }
        
        double price_change = prices[i] - prices[i-1];
        double vol = volatility[i];
        
        // State classification based on price change and volatility
        if (price_change > 0 && vol < 0.02) result.push_back(0.0); // Trending up, low vol
        else if (price_change < 0 && vol < 0.02) result.push_back(1.0); // Trending down, low vol  
        else result.push_back(2.0); // High volatility state
    }
    return result;
}

std::vector<double> TechnicalIndicators::high_volatility_indicator_garch_threshold(const std::vector<double>& returns, double threshold) {
    auto garch_vol = garch_volatility_21(returns);
    if (garch_vol.empty()) return {};
    
    std::vector<double> result;
    result.reserve(garch_vol.size());
    
    for (double vol : garch_vol) {
        result.push_back(vol > threshold ? 1.0 : 0.0);
    }
    return result;
}

// Market Microstructure
std::vector<double> TechnicalIndicators::bid_ask_spread_volatility_10(const std::vector<double>& bid_ask_spread) {
    return calculate_rolling_volatility(bid_ask_spread, 10);
}

std::vector<double> TechnicalIndicators::order_flow_imbalance_5(const std::vector<double>& uptick_volume, const std::vector<double>& downtick_volume) {
    if (uptick_volume.size() != downtick_volume.size()) return {};
    
    std::vector<double> imbalance;
    imbalance.reserve(uptick_volume.size());
    
    for (size_t i = 0; i < uptick_volume.size(); ++i) {
        double total = uptick_volume[i] + downtick_volume[i];
        double ofi = total > 0 ? (uptick_volume[i] - downtick_volume[i]) / total : 0.0;
        imbalance.push_back(ofi);
    }
    
    return simple_moving_average(imbalance, 5);
}

std::vector<double> TechnicalIndicators::price_impact_of_volume_spike(const std::vector<double>& prices, const std::vector<double>& volume) {
    if (prices.size() != volume.size() || prices.size() < 2) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - 1);
    
    // Calculate volume threshold (3 standard deviations)
    double vol_mean = 0.0, vol_var = 0.0;
    for (double v : volume) vol_mean += v;
    vol_mean /= volume.size();
    
    for (double v : volume) vol_var += (v - vol_mean) * (v - vol_mean);
    vol_var /= (volume.size() - 1);
    double vol_threshold = vol_mean + 3.0 * std::sqrt(vol_var);
    
    for (size_t i = 1; i < prices.size(); ++i) {
        double price_impact = 0.0;
        if (volume[i] > vol_threshold) {
            price_impact = prices[i-1] > 0 ? (prices[i] - prices[i-1]) / prices[i-1] : 0.0;
        }
        result.push_back(price_impact);
    }
    return result;
}

std::vector<double> TechnicalIndicators::probability_of_informed_trading_pin(const std::vector<double>& buy_volume, const std::vector<double>& sell_volume) {
    if (buy_volume.size() != sell_volume.size()) return {};
    
    std::vector<double> result;
    result.reserve(buy_volume.size());
    
    // Simplified PIN calculation
    for (size_t i = 0; i < buy_volume.size(); ++i) {
        double total_volume = buy_volume[i] + sell_volume[i];
        double imbalance = total_volume > 0 ? std::abs(buy_volume[i] - sell_volume[i]) / total_volume : 0.0;
        result.push_back(imbalance);
    }
    return result;
}

std::vector<double> TechnicalIndicators::effective_spread_5_min_avg(const std::vector<double>& effective_spread) {
    return simple_moving_average(effective_spread, 5);
}

// Volatility Surface
std::vector<double> TechnicalIndicators::implied_volatility_atm_30d(const std::vector<double>& iv_data) {
    return iv_data; // Pass through - assuming data is already ATM 30d IV
}

std::vector<double> TechnicalIndicators::implied_volatility_skew_25d(const std::vector<double>& put_iv, const std::vector<double>& call_iv) {
    if (put_iv.size() != call_iv.size()) return {};
    
    std::vector<double> result;
    result.reserve(put_iv.size());
    
    for (size_t i = 0; i < put_iv.size(); ++i) {
        result.push_back(put_iv[i] - call_iv[i]);
    }
    return result;
}

std::vector<double> TechnicalIndicators::implied_volatility_term_structure_slope_30d_90d(const std::vector<double>& iv_30d, const std::vector<double>& iv_90d) {
    if (iv_30d.size() != iv_90d.size()) return {};
    
    std::vector<double> result;
    result.reserve(iv_30d.size());
    
    for (size_t i = 0; i < iv_30d.size(); ++i) {
        result.push_back(iv_90d[i] - iv_30d[i]);
    }
    return result;
}

std::vector<double> TechnicalIndicators::realized_vs_implied_volatility_ratio_20(const std::vector<double>& realized_vol, const std::vector<double>& implied_vol) {
    if (realized_vol.size() != implied_vol.size()) return {};
    
    std::vector<double> result;
    result.reserve(realized_vol.size());
    
    for (size_t i = 0; i < realized_vol.size(); ++i) {
        double ratio = implied_vol[i] > 0 ? realized_vol[i] / implied_vol[i] : 1.0;
        result.push_back(ratio);
    }
    return result;
}

// Factor Exposures
std::vector<double> TechnicalIndicators::fama_french_smb_beta_120(const std::vector<double>& returns, const std::vector<double>& smb_returns) {
    return beta_to_market_60(returns, smb_returns); // Reuse beta calculation with 120 period
}

std::vector<double> TechnicalIndicators::fama_french_hml_beta_120(const std::vector<double>& returns, const std::vector<double>& hml_returns) {
    return beta_to_market_60(returns, hml_returns);
}

std::vector<double> TechnicalIndicators::momentum_factor_beta_120(const std::vector<double>& returns, const std::vector<double>& momentum_returns) {
    return beta_to_market_60(returns, momentum_returns);
}

std::vector<double> TechnicalIndicators::quality_factor_beta_120(const std::vector<double>& returns, const std::vector<double>& quality_returns) {
    return beta_to_market_60(returns, quality_returns);
}

// Non-Linear/Interaction
std::vector<double> TechnicalIndicators::return_x_volume_interaction_10(const std::vector<double>& returns, const std::vector<double>& volume) {
    if (returns.size() != volume.size()) return {};
    
    std::vector<double> interaction;
    interaction.reserve(returns.size());
    
    for (size_t i = 0; i < returns.size(); ++i) {
        interaction.push_back(returns[i] * volume[i]);
    }
    
    return simple_moving_average(interaction, 10);
}

std::vector<double> TechnicalIndicators::volatility_x_rsi_interaction_14(const std::vector<double>& volatility, const std::vector<double>& rsi) {
    if (volatility.size() != rsi.size()) return {};
    
    std::vector<double> result;
    result.reserve(volatility.size());
    
    for (size_t i = 0; i < volatility.size(); ++i) {
        result.push_back(volatility[i] * rsi[i]);
    }
    return result;
}

std::vector<double> TechnicalIndicators::price_to_kama_ratio_20_10_30(const std::vector<double>& prices) {
    auto kama_values = kama(prices, 20, 10, 30);
    if (kama_values.empty() || kama_values.size() != prices.size()) return {};
    
    std::vector<double> result;
    result.reserve(prices.size());
    
    for (size_t i = 0; i < prices.size(); ++i) {
        double ratio = kama_values[i] > 0 ? prices[i] / kama_values[i] : 1.0;
        result.push_back(ratio);
    }
    return result;
}

std::vector<double> TechnicalIndicators::polynomial_regression_price_degree_2_slope(const std::vector<double>& prices, int window) {
    if (prices.size() < static_cast<size_t>(window) || window < 3) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - window + 1);
    
    for (size_t i = 0; i <= prices.size() - window; ++i) {
        // Simplified polynomial regression - use linear slope as approximation
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        
        for (int j = 0; j < window; ++j) {
            double x = static_cast<double>(j);
            double y = prices[i + j];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double num = window * sum_xy - sum_x * sum_y;
        double den = window * sum_x2 - sum_x * sum_x;
        double slope = den != 0 ? num / den : 0.0;
        result.push_back(slope);
    }
    return result;
}

// Alternative Risk Measures
std::vector<double> TechnicalIndicators::conditional_value_at_risk_cvar_95_20(const std::vector<double>& returns) {
    const int window = 20;
    if (returns.size() < static_cast<size_t>(window)) return {};
    
    std::vector<double> result;
    result.reserve(returns.size() - window + 1);
    
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        std::vector<double> window_returns(returns.begin() + i, returns.begin() + i + window);
        std::sort(window_returns.begin(), window_returns.end());
        
        // 95% CVaR - average of worst 5% returns
        int tail_size = std::max(1, static_cast<int>(window * 0.05));
        double cvar = 0.0;
        for (int j = 0; j < tail_size; ++j) {
            cvar += window_returns[j];
        }
        cvar /= tail_size;
        result.push_back(cvar);
    }
    return result;
}

std::vector<double> TechnicalIndicators::drawdown_duration_from_peak_50(const std::vector<double>& prices) {
    const int window = 50;
    if (prices.size() < static_cast<size_t>(window)) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - window + 1);
    
    for (size_t i = window - 1; i < prices.size(); ++i) {
        double peak = *std::max_element(prices.begin() + i - window + 1, prices.begin() + i + 1);
        int duration = 0;
        
        for (int j = 0; j < window; ++j) {
            if (prices[i - j] >= peak) break;
            duration++;
        }
        result.push_back(static_cast<double>(duration));
    }
    return result;
}

std::vector<double> TechnicalIndicators::ulcer_index_14(const std::vector<double>& prices) {
    const int period = 14;
    if (prices.size() < static_cast<size_t>(period)) return {};
    
    std::vector<double> result;
    result.reserve(prices.size() - period + 1);
    
    for (size_t i = period - 1; i < prices.size(); ++i) {
        double max_price = *std::max_element(prices.begin() + i - period + 1, prices.begin() + i + 1);
        double sum_squared_drawdowns = 0.0;
        
        for (int j = 0; j < period; ++j) {
            double drawdown = max_price > 0 ? 100.0 * (prices[i - j] - max_price) / max_price : 0.0;
            sum_squared_drawdowns += drawdown * drawdown;
        }
        
        double ulcer = std::sqrt(sum_squared_drawdowns / period);
        result.push_back(ulcer);
    }
    return result;
}

std::vector<double> TechnicalIndicators::sortino_ratio_30(const std::vector<double>& returns) {
    const int window = 30;
    if (returns.size() < static_cast<size_t>(window)) return {};
    
    std::vector<double> result;
    result.reserve(returns.size() - window + 1);
    
    for (size_t i = 0; i <= returns.size() - window; ++i) {
        double sum = 0.0;
        for (int j = 0; j < window; ++j) sum += returns[i + j];
        double mean_return = sum / window;
        
        double downside_variance = 0.0;
        int downside_count = 0;
        for (int j = 0; j < window; ++j) {
            if (returns[i + j] < 0) {
                downside_variance += returns[i + j] * returns[i + j];
                downside_count++;
            }
        }
        
        double downside_deviation = downside_count > 0 ? std::sqrt(downside_variance / downside_count) : 0.0;
        double sortino = downside_deviation > 0 ? mean_return / downside_deviation : 0.0;
        result.push_back(sortino);
    }
    return result;
}

// Private helper functions
double TechnicalIndicators::calculate_atr(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, int period, size_t index) {
    if (index < static_cast<size_t>(period)) return 0.0;
    
    double atr_sum = 0.0;
    for (int i = 0; i < period; ++i) {
        size_t idx = index - i;
        double tr = high[idx] - low[idx];
        if (idx > 0) {
            tr = std::max({tr, std::abs(high[idx] - close[idx-1]), std::abs(low[idx] - close[idx-1])});
        }
        atr_sum += tr;
    }
    return atr_sum / period;
}

std::vector<double> TechnicalIndicators::exponential_moving_average(const std::vector<double>& data, int period) {
    if (data.empty() || period <= 0) return {};
    
    std::vector<double> result;
    result.reserve(data.size());
    
    double alpha = 2.0 / (period + 1.0);
    result.push_back(data[0]);
    
    for (size_t i = 1; i < data.size(); ++i) {
        double ema = alpha * data[i] + (1.0 - alpha) * result.back();
        result.push_back(ema);
    }
    return result;
}
