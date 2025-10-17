#pragma once
#include <vector>
#include <chrono>

namespace fingraph {

// Forward declaration to avoid including Portfolio.h
class Trade; 

class PerformanceMetrics {
public:
    // Calculates the Sharpe Ratio. riskFreeRate is annualized.
    static double calculateSharpeRatio(
        const std::vector<std::pair<std::chrono::system_clock::time_point, double> >& equityCurve,
        double riskFreeRate = 0.0);

    // Calculates the Maximum Drawdown.
    static double calculateMaxDrawdown(
        const std::vector<std::pair<std::chrono::system_clock::time_point, double> >& equityCurve);

    // Calculates the percentage of profitable trades.
    static double calculateWinRate(const std::vector<Trade>& trades);

    // Calculates the total return of the backtest.
    static double calculateTotalReturn(
        const std::vector<std::pair<std::chrono::system_clock::time_point, double> >& equityCurve);
};

} // namespace fingraph