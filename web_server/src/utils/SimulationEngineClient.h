#pragma once
#include <string>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SimulationEngineClient {
public:
    SimulationEngineClient(std::string enginePath);
    ~SimulationEngineClient() = default;
    
    json runBacktest(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
    
private:
    std::string enginePath_;
    
    static std::string createConfigFile(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
};