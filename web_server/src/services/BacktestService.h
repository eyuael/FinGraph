#pragma once
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "../utils/SimulationEngineClient.h"

using json = nlohmann::json;
namespace fingraph {
class BacktestService {
public:
    BacktestService(const std::string& simulationEnginePath, const std::string& dataDirectory);
    ~BacktestService() = default;
    
    json runBacktest(
        const std::string& dataId,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
    
    static std::vector<std::string> getAvailableStrategies();
    
private:
    std::string dataDirectory_;
    SimulationEngineClient engineClient_;
    std::map<std::string, std::string> dataIdToPath_;
    
    void loadDataMapping();
};//namespace fingraph
}