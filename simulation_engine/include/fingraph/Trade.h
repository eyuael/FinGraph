#pragma once
#include <string>
#include <chrono>

namespace fingraph {

enum class TradeType { BUY, SELL };

class Trade {
public:
    Trade(const std::string& symbol, TradeType type, double quantity, double price,
          const std::chrono::system_clock::time_point& timestamp);

    // Getters for all trade properties
    const std::string& getSymbol() const { return symbol_; }
    TradeType getType() const { return type_; }
    double getQuantity() const { return quantity_; }
    double getPrice() const { return price_; }
    const std::chrono::system_clock::time_point& getTimestamp() const { return timestamp_; }
    double getValue() const { return quantity_ * price_; }

private:
    std::string symbol_;
    TradeType type_;
    double quantity_;
    double price_;
    std::chrono::system_clock::time_point timestamp_;
};
}