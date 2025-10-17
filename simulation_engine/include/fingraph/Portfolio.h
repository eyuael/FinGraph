#pragma once
#include "Trade.h"
#include <vector>
#include <map>
#include <string>

namespace fingraph {

class Portfolio {
public:
    explicit Portfolio(double initialCash);

    // Executes a trade, updating cash and positions.
    void addTrade(const Trade& trade);

    double getCash() const { return cash_; }
    // Returns the quantity of shares held for a given symbol.
    double getPosition(const std::string& symbol) const;
    
    // Calculates the total value of all held positions at current market prices.
    double getEquityValue(const std::map<std::string, double>& currentPrices) const;
    
    // Total portfolio value = cash + equity value.
    double getTotalValue(const std::map<std::string, double>& currentPrices) const;
    
    const std::vector<Trade>& getTrades() const { return trades_; }

private:
    double cash_;
    // Maps a symbol (e.g., "AAPL") to the quantity of shares held.
    std::map<std::string, double> positions_;
    std::vector<Trade> trades_;
};
}