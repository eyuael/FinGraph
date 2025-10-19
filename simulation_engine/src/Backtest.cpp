#include "fingraph/Backtest.h"
#include "fingraph/MarketData.h"
#include "fingraph/Portfolio.h"
#include "fingraph/PerformanceMetrics.h"
#include "fingraph/strategies/MovingAverageStrategy.h"
#include "fingraph/strategies/RSIStrategy.h"
#include <memory>
#include <stdexcept>
#include <map>

namespace fingraph {

BacktestEngine::BacktestEngine() {
    initializeStrategies();
}

void BacktestEngine::initializeStrategies() {
    // Register all available strategies here
    strategies_["Moving Average Crossover"] = std::make_unique<MovingAverageStrategy>();
    strategies_["RSI Mean Reversion"] = std::make_unique<RSIStrategy>();
}

Strategy* BacktestEngine::getStrategy(const std::string& name) {
    auto it = strategies_.find(name);
    if (it != strategies_.end()) {
        return it->second.get();
    }
    throw std::invalid_argument("Strategy not found: " + name);
}

std::vector<std::string> BacktestEngine::getAvailableStrategies() const {
    std::vector<std::string> names;
    for (const auto& pair : strategies_) {
        names.push_back(pair.first);
    }
    return names;
}

BacktestResult BacktestEngine::runBacktest(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {

    // 1. Setup
    MarketData marketData;
    if (!marketData.loadFromCSV(dataPath)) {
        throw std::runtime_error("Failed to load market data from " + dataPath);
    }

    Strategy* strategy = getStrategy(strategyName);
    strategy->updateParameters(strategyParams);
    
    const auto& data = marketData.getData();
    strategy->initialize(data); // Pre-calculate indicators

    Portfolio portfolio(initialCash);
    BacktestResult result;
    
    // 2. Simulation Loop
    for (size_t i = 0; i < data.size(); ++i) {
        const auto& candle = data[i];
        
        // Generate signal
        Signal signal = strategy->generateSignal(i);

        // Execute trade based on signal
        if (signal == Signal::BUY && portfolio.getPosition("DEFAULT") == 0) { // Simple logic: one open position
            double quantity = std::floor(portfolio.getCash() / candle.close); // All-in
            if (quantity > 0) {
                Trade trade("DEFAULT", TradeType::BUY, quantity, candle.close, candle.timestamp);
                portfolio.addTrade(trade);
            }
        } else if (signal == Signal::SELL && portfolio.getPosition("DEFAULT") > 0) {
            double quantity = portfolio.getPosition("DEFAULT");
            Trade trade("DEFAULT", TradeType::SELL, quantity, candle.close, candle.timestamp);
            portfolio.addTrade(trade);
        }
        
        // 3. Record Equity Curve
        std::map<std::string, double> currentPrices = { {"DEFAULT", candle.close} };
        double totalValue = portfolio.getTotalValue(currentPrices);
        result.equityCurve.emplace_back(candle.timestamp, totalValue);
    }

    // 4. Finalize Results
    result.trades = portfolio.getTrades();
    result.totalReturn = PerformanceMetrics::calculateTotalReturn(result.equityCurve);
    result.maxDrawdown = PerformanceMetrics::calculateMaxDrawdown(result.equityCurve);
    result.sharpeRatio = PerformanceMetrics::calculateSharpeRatio(result.equityCurve);
    result.winRate = PerformanceMetrics::calculateWinRate(result.trades);

    return result;
}

} // namespace fingraph