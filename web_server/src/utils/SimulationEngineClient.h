#pragma once
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SimulationEngineClient {
public:
    SimulationEngineClient(const std::string& server_address);
    ~SimulationEngineClient() = default;
    
    // Job management methods
    std::string submitBacktest(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
    
    json getJobStatus(const std::string& job_id);
    json getJobResults(const std::string& job_id);
    bool cancelJob(const std::string& job_id);
    
    // Strategy information methods
    json listStrategies();
    json getStrategyParameters(const std::string& strategy_name);
    
    // Legacy method for backward compatibility
    json runBacktest(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
    
private:
    std::string server_address_;
    
    // gRPC client stub (will be initialized when gRPC is available)
    void* grpc_stub_; // Will be std::unique_ptr<SimulationEngine::Stub>
    
    // Fallback to CLI if gRPC is not available
    std::string enginePath_;
    bool use_grpc_;
    
    // Legacy CLI methods
    json runBacktestCLI(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
    
    static std::string createConfigFile(
        const std::string& dataPath,
        const std::string& strategyName,
        const std::map<std::string, double>& strategyParams,
        double initialCash
    );
};