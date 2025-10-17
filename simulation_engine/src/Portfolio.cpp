#include "fingraph/Portfolio.h"
#include "fingraph/Trade.h"
#include <iostream>

namespace fingraph {

Portfolio::Portfolio(double initialCash) : cash_(initialCash) {}

void Portfolio::addTrade(const Trade& trade) {
    trades_.push_back(trade);
    
    double tradeValue = trade.getValue();
    
    if (trade.getType() == TradeType::BUY) {
        // For a BUY, we spend cash and gain a position
        if (cash_ < tradeValue) {
            throw std::runtime_error("Insufficient cash for trade.");
        }
        cash_ -= tradeValue;
        positions_[trade.getSymbol()] += trade.getQuantity();
    } else { // SELL
        // For a SELL, we gain cash and reduce a position
        if (getPosition(trade.getSymbol()) < trade.getQuantity()) {
            throw std::runtime_error("Insufficient position for sell trade.");
        }
        cash_ += tradeValue;
        positions_[trade.getSymbol()] -= trade.getQuantity();
    }
}

double Portfolio::getPosition(const std::string& symbol) const {
    auto it = positions_.find(symbol);
    if (it != positions_.end()) {
        return it->second;
    }
    return 0.0; // Return 0 if no position is held
}

double Portfolio::getEquityValue(const std::map<std::string, double>& currentPrices) const {
    double totalEquity = 0.0;
    for (const auto& pair : positions_) {
        const std::string& symbol = pair.first;
        double quantity = pair.second;
        
        auto priceIt = currentPrices.find(symbol);
        if (priceIt != currentPrices.end()) {
            totalEquity += quantity * priceIt->second;
        }
    }
    return totalEquity;
}

double Portfolio::getTotalValue(const std::map<std::string, double>& currentPrices) const {
    return cash_ + getEquityValue(currentPrices);
}

} // namespace fingraph