#include "services/BacktestService.h"
#include "utils/SimulationEngineClient.h"
#include <stdexcept>
#include <filesystem>

namespace fingraph {

BacktestService::BacktestService(const std::string& simulationEnginePath, const std::string& dataDirectory)
    : engineClient_(simulationEnginePath), dataDirectory_(dataDirectory) {
    // Ensure the data directory exists
    std::filesystem::create_directories(dataDirectory_);
}

json BacktestService::runBacktest(
    const std::string& dataId,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    // 1. Resolve dataId to a full path
    std::string dataPath = dataDirectory_ + "/" + dataId;
    if (!std::filesystem::exists(dataPath)) {
        throw std::invalid_argument("Data file not found for ID: " + dataId);
    }

    // 2. Call the simulation engine client
    try {
        json result = engineClient_.runBacktest(dataPath, strategyName, strategyParams, initialCash);
        return result;
    } catch (const std::exception& e) {
        // Wrap the engine's exception in a more descriptive one
        throw std::runtime_error("Simulation engine failed: " + std::string(e.what()));
    }
}

 std::vector<std::string> BacktestService::getAvailableStrategies() {
    // This is a bit of a hack. A better way would be for the engine to have a
    // `--list-strategies` CLI flag. For now, we hardcode it.
    // This must match the strategies in the simulation engine.
    return {"Moving Average Crossover", "RSI Mean Reversion"};
}

} // namespace fingraph