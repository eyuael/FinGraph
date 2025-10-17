#pragma once

#include "fingraph/MarketData.h"
#include "fingraph/Strategy.h"
#include "fingraph/Portfolio.h"
#include "fingraph/PerformanceMetrics.h"
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace fingraph {

struct BacktestResult {
    double totalReturn = 0.0;
    double sharpeRatio = 0.0;
    double maxDrawdown = 0.0;
    double winRate = 0.0;
    std::vector<Trade> trades;
    // A time-series of the total portfolio value.
    std::vector<std::pair<std::chrono::system_clock::time_point, double> > equityCurve;
};

class BacktestEngine {
public:
    BacktestEngine();
    ~BacktestEngine() = default;

    // Main method to run a backtest.
    BacktestResult runBacktest(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );

    // Returns a list of available strategy names.
    std::vector<std::string> getAvailableStrategies() const;

private:
    // Factory to create strategy instances by name.
    std::map<std::string, std::unique_ptr<Strategy> > strategies_;

    void initializeStrategies();
    Strategy* getStrategy(const std::string& name);
};

} // namespace fingraph