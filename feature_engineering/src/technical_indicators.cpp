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
