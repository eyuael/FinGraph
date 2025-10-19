#include "fingraph/strategies/RSIStrategy.h"
#include <vector>
#include <numeric>

namespace fingraph {

RSIStrategy::RSIStrategy()
    : Strategy("RSI Mean Reversion"), period_(14), oversoldThreshold_(30.0), overboughtThreshold_(70.0) {}

void RSIStrategy::initialize(const std::vector<OHLCV>& data) {
    if (data.size() < period_) {
        throw std::invalid_argument("Not enough data for RSI calculation.");
    }
    
    calculateRSI(data);
}

Signal RSIStrategy::generateSignal(size_t index) const {
    if (index < period_ || index >= rsiValues_.size()) {
        return Signal::NONE;
    }
    
    double rsi = rsiValues_[index];
    
    if (rsi <= oversoldThreshold_) {
        return Signal::BUY;
    } else if (rsi >= overboughtThreshold_) {
        return Signal::SELL;
    }
    
    return Signal::NONE;
}

void RSIStrategy::updateParameters(const std::map<std::string, double>& params) {
    auto it = params.find("period");
    if (it != params.end()) {
        period_ = static_cast<size_t>(it->second);
    }
    
    it = params.find("oversoldThreshold");
    if (it != params.end()) {
        oversoldThreshold_ = it->second;
    }
    
    it = params.find("overboughtThreshold");
    if (it != params.end()) {
        overboughtThreshold_ = it->second;
    }
    
    // Note: initialize() must be called again after updating parameters
}

void RSIStrategy::calculateRSI(const std::vector<OHLCV>& data) {
    rsiValues_.clear();
    rsiValues_.resize(data.size(), 0.0);
    
    std::vector<double> gains(data.size(), 0.0);
    std::vector<double> losses(data.size(), 0.0);
    
    // Calculate price changes
    for (size_t i = 1; i < data.size(); ++i) {
        double change = data[i].close - data[i-1].close;
        if (change > 0) {
            gains[i] = change;
        } else {
            losses[i] = -change;
        }
    }
    
    // Calculate initial averages
    for (size_t i = period_; i < data.size(); ++i) {
        double avgGain = std::accumulate(gains.begin() + i - period_ + 1, gains.begin() + i + 1, 0.0) / period_;
        double avgLoss = std::accumulate(losses.begin() + i - period_ + 1, losses.begin() + i + 1, 0.0) / period_;
        
        double rs = (avgLoss == 0) ? 100.0 : avgGain / avgLoss;
        rsiValues_[i] = 100.0 - (100.0 / (1.0 + rs));
    }
}

} // namespace fingraph