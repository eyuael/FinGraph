#include "fingraph/strategies/MovingAverageStrategy.h"
#include <numeric>
#include <stdexcept>

namespace fingraph {

MovingAverageStrategy::MovingAverageStrategy() 
    : Strategy("Moving Average Crossover"), shortPeriod_(10), longPeriod_(30) {}

void MovingAverageStrategy::initialize(const std::vector<OHLCV>& data) {
    if (data.size() < longPeriod_) {
        throw std::invalid_argument("Not enough data for long-period moving average.");
    }
    
    // Pre-calculate all moving averages to speed up signal generation
    calculateMovingAverages(data);
}

Signal MovingAverageStrategy::generateSignal(size_t index) const {
    // Cannot generate a signal before the long MA is fully calculated
    if (index < longPeriod_ || index >= shortMA_.size()) {
        return Signal::NONE;
    }
    
    // Check for a bullish crossover (short MA crosses above long MA)
    bool wasBelow = shortMA_[index - 1] < longMA_[index - 1];
    bool isAbove = shortMA_[index] > longMA_[index];
    
    if (wasBelow && isAbove) {
        return Signal::BUY;
    }
    
    // Check for a bearish crossover (short MA crosses below long MA)
    bool wasAbove = shortMA_[index - 1] > longMA_[index - 1];
    bool isBelow = shortMA_[index] < longMA_[index];
    
    if (wasAbove && isBelow) {
        return Signal::SELL;
    }
    
    return Signal::NONE;
}

void MovingAverageStrategy::updateParameters(const std::map<std::string, double>& params) {
    auto it = params.find("shortPeriod");
    if (it != params.end()) {
        shortPeriod_ = static_cast<size_t>(it->second);
    }
    
    it = params.find("longPeriod");
    if (it != params.end()) {
        longPeriod_ = static_cast<size_t>(it->second);
    }
    
    // Note: initialize() must be called again after updating parameters
}

void MovingAverageStrategy::calculateMovingAverages(const std::vector<OHLCV>& data) {
    shortMA_.clear();
    longMA_.clear();
    
    // Use closing prices for calculation
    std::vector<double> closes;
    for (const auto& candle : data) {
        closes.push_back(candle.close);
    }

    // Calculate Simple Moving Average (SMA)
    for (size_t i = 0; i < closes.size(); ++i) {
        if (i + 1 >= shortPeriod_) {
            double sum = std::accumulate(closes.end() - shortPeriod_, closes.end(), 0.0);
            shortMA_.push_back(sum / shortPeriod_);
        } else {
            shortMA_.push_back(0.0); // Not enough data yet
        }
        
        if (i + 1 >= longPeriod_) {
            double sum = std::accumulate(closes.end() - longPeriod_, closes.end(), 0.0);
            longMA_.push_back(sum / longPeriod_);
        } else {
            longMA_.push_back(0.0); // Not enough data yet
        }
    }
}

} // namespace fingraph