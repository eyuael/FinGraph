#include "fingraph/Trade.h"

fingraph::Trade::Trade(const std::string& symbol, TradeType type, double quantity, double price,
             const std::chrono::system_clock::time_point& timestamp)
    : symbol_(symbol), type_(type), quantity_(quantity), price_(price), timestamp_(timestamp) {
    // Constructor body can be empty due to member initializer list
}

// namespace fingraph