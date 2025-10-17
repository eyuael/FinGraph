#include "fingraph/PerformanceMetrics.h"
#include "fingraph/Trade.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <map>

namespace fingraph {

double PerformanceMetrics::calculateTotalReturn(
    const std::vector<std::pair<std::chrono::system_clock::time_point, double>>& equityCurve) {
    if (equityCurve.empty()) return 0.0;
    
    double initialValue = equityCurve.front().second;
    double finalValue = equityCurve.back().second;
    
    if (initialValue == 0) return 0.0; // Avoid division by zero
    
    return (finalValue - initialValue) / initialValue;
}

double PerformanceMetrics::calculateMaxDrawdown(
    const std::vector<std::pair<std::chrono::system_clock::time_point, double>>& equityCurve) {
    if (equityCurve.empty()) return 0.0;

    double maxDrawdown = 0.0;
    double peak = equityCurve.front().second;
    
    for (const auto& point : equityCurve) {
        double currentValue = point.second;
        if (currentValue > peak) {
            peak = currentValue; // Found a new peak
        }
        
        double drawdown = (peak - currentValue) / peak;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown; // Found a new max drawdown
        }
    }
    
    return maxDrawdown;
}

double PerformanceMetrics::calculateWinRate(const std::vector<Trade>& trades) {
    if (trades.empty() || trades.size() < 2) return 0.0;

    int profitableTrades = 0;
    // We need to pair buys and sells to calculate profit/loss
    std::map<std::string, double> openPositions; // symbol -> buy price

    for (const auto& trade : trades) {
        if (trade.getType() == TradeType::BUY) {
            openPositions[trade.getSymbol()] = trade.getPrice();
        } else { // SELL
            auto it = openPositions.find(trade.getSymbol());
            if (it != openPositions.end()) {
                if (trade.getPrice() > it->second) {
                    profitableTrades++;
                }
                openPositions.erase(it);
            }
        }
    }
    
    // Number of completed trades is half the total number of trades (buy/sell pairs)
    int completedTrades = trades.size() / 2;
    if (completedTrades == 0) return 0.0;

    return static_cast<double>(profitableTrades) / completedTrades;
}

double PerformanceMetrics::calculateSharpeRatio(
    const std::vector<std::pair<std::chrono::system_clock::time_point, double>>& equityCurve,
    double riskFreeRate) {
    if (equityCurve.size() < 2) return 0.0;

    // 1. Calculate daily returns
    std::vector<double> returns;
    for (size_t i = 1; i < equityCurve.size(); ++i) {
        double prevValue = equityCurve[i-1].second;
        double currValue = equityCurve[i].second;
        if (prevValue != 0) {
            returns.push_back((currValue - prevValue) / prevValue);
        }
    }

    if (returns.empty()) return 0.0;

    // 2. Calculate average return and standard deviation of returns
    double meanReturn = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    
    double variance = 0.0;
    for (double r : returns) {
        variance += (r - meanReturn) * (r - meanReturn);
    }
    double stdDev = std::sqrt(variance / returns.size());

    // 3. Annualize and calculate Sharpe Ratio
    // Assuming daily data, 252 trading days in a year
    double annualizedMeanReturn = meanReturn * 252;
    double annualizedStdDev = stdDev * std::sqrt(252);
    
    if (annualizedStdDev == 0) return 0.0; // Avoid division by zero

    return (annualizedMeanReturn - riskFreeRate) / annualizedStdDev;
}

} // namespace fingraph