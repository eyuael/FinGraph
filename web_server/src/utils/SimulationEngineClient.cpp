#include "SimulationEngineClient.h"
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <array>
#include <iostream>
#include <thread>
#include <chrono>

// Placeholder structures for protobuf types (will be replaced by generated classes)
namespace fingraph {
    struct BacktestRequest {
        std::string data_path;
        std::string strategy_name;
        std::map<std::string, double> strategy_params;
        double initial_cash;
        std::string job_id;
    };
    
    struct JobResponse {
        std::string job_id;
        int status;
        std::string message;
    };
    
    struct JobStatusRequest {
        std::string job_id;
    };
    
    struct JobStatusResponse {
        std::string job_id;
        int status;
        double progress;
        std::string message;
        int64_t start_time;
        int64_t estimated_completion;
    };
    
    struct JobResultsRequest {
        std::string job_id;
    };
    
    struct BacktestResults {
        std::string job_id;
        double total_return;
        double sharpe_ratio;
        double max_drawdown;
        double win_rate;
        std::map<std::string, std::string> trades; // Simplified
        std::map<int64_t, double> equity_curve; // Simplified
    };
    
    struct CancelJobRequest {
        std::string job_id;
    };
    
    struct CancelJobResponse {
        bool success;
        std::string message;
    };
    
    struct ListStrategiesRequest {};
    
    struct ListStrategiesResponse {
        std::vector<std::string> strategies;
    };
    
    struct StrategyParamsRequest {
        std::string strategy_name;
    };
    
    struct StrategyParamsResponse {
        std::map<std::string, std::string> parameters;
    };
}

SimulationEngineClient::SimulationEngineClient(const std::string& server_address)
    : server_address_(server_address)
    , grpc_stub_(nullptr)
    , use_grpc_(false) {
    
    // Check if this is a gRPC address or CLI path
    if (server_address.find(":") != std::string::npos) {
        // This looks like a gRPC address (host:port)
        use_grpc_ = true;
        std::cout << "Initializing gRPC client for server: " << server_address_ << std::endl;
        // TODO: Initialize actual gRPC stub here
        // grpc_stub_ = SimulationEngine::NewStub(grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials()));
    } else {
        // This is a CLI path
        enginePath_ = server_address;
        use_grpc_ = false;
        std::cout << "Initializing CLI client for engine: " << enginePath_ << std::endl;
    }
}

std::string SimulationEngineClient::submitBacktest(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        std::cout << "Submitting backtest via gRPC (not yet implemented)" << std::endl;
        return "grpc_job_" + std::to_string(std::time(nullptr));
    } else {
        // Fallback to CLI mode - run synchronously and return a job ID
        json result = runBacktestCLI(dataPath, strategyName, strategyParams, initialCash);
        return "cli_job_" + std::to_string(std::time(nullptr));
    }
}

json SimulationEngineClient::getJobStatus(const std::string& job_id) {
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        json status;
        status["job_id"] = job_id;
        status["status"] = "COMPLETED";
        status["progress"] = 1.0;
        status["message"] = "Job completed (gRPC mock)";
        return status;
    } else {
        // CLI mode - jobs are synchronous, so if we have a job ID it's completed
        json status;
        status["job_id"] = job_id;
        status["status"] = "COMPLETED";
        status["progress"] = 1.0;
        status["message"] = "Job completed (CLI mode)";
        return status;
    }
}

json SimulationEngineClient::getJobResults(const std::string& job_id) {
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        json results;
        results["job_id"] = job_id;
        results["total_return"] = 0.15;
        results["sharpe_ratio"] = 1.2;
        results["max_drawdown"] = 0.05;
        results["win_rate"] = 0.6;
        results["trades"] = json::array();
        results["equity_curve"] = json::array();
        return results;
    } else {
        // CLI mode - return empty results for now
        json results;
        results["job_id"] = job_id;
        results["total_return"] = 0.0;
        results["sharpe_ratio"] = 0.0;
        results["max_drawdown"] = 0.0;
        results["win_rate"] = 0.0;
        results["trades"] = json::array();
        results["equity_curve"] = json::array();
        return results;
    }
}

bool SimulationEngineClient::cancelJob(const std::string& job_id) {
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        std::cout << "Cancelling job via gRPC: " << job_id << std::endl;
        return true;
    } else {
        // CLI mode - can't cancel running jobs
        std::cout << "Cannot cancel job in CLI mode: " << job_id << std::endl;
        return false;
    }
}

json SimulationEngineClient::listStrategies() {
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        json strategies = json::array();
        strategies.push_back({{"name", "MovingAverage"}, {"description", "Moving Average Crossover Strategy"}});
        strategies.push_back({{"name", "RSI"}, {"description", "Relative Strength Index Strategy"}});
        return strategies;
    } else {
        // CLI mode - return hardcoded strategies
        json strategies = json::array();
        strategies.push_back({{"name", "MovingAverage"}, {"description", "Moving Average Crossover Strategy"}});
        strategies.push_back({{"name", "RSI"}, {"description", "Relative Strength Index Strategy"}});
        return strategies;
    }
}

json SimulationEngineClient::getStrategyParameters(const std::string& strategy_name) {
    if (use_grpc_) {
        // TODO: Implement actual gRPC call
        json parameters = json::object();
        if (strategy_name == "MovingAverage") {
            parameters["short_window"] = 10;
            parameters["long_window"] = 20;
        } else if (strategy_name == "RSI") {
            parameters["period"] = 14;
            parameters["overbought_threshold"] = 70;
            parameters["oversold_threshold"] = 30;
        }
        return parameters;
    } else {
        // CLI mode - return hardcoded parameters
        json parameters = json::object();
        if (strategy_name == "MovingAverage") {
            parameters["short_window"] = 10;
            parameters["long_window"] = 20;
        } else if (strategy_name == "RSI") {
            parameters["period"] = 14;
            parameters["overbought_threshold"] = 70;
            parameters["oversold_threshold"] = 30;
        }
        return parameters;
    }
}

json SimulationEngineClient::runBacktest(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    if (use_grpc_) {
        // Submit job and wait for completion (for backward compatibility)
        std::string job_id = submitBacktest(dataPath, strategyName, strategyParams, initialCash);
        
        // Poll for completion
        while (true) {
            json status = getJobStatus(job_id);
            if (status["status"] == "COMPLETED" || status["status"] == "FAILED") {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        return getJobResults(job_id);
    } else {
        // Use CLI mode
        return runBacktestCLI(dataPath, strategyName, strategyParams, initialCash);
    }
}

json SimulationEngineClient::runBacktestCLI(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    // Create temporary config file
    std::string configFile = createConfigFile(dataPath, strategyName, strategyParams, initialCash);
    
    // Run simulation engine
    std::string command = enginePath_ + " " + configFile;
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Clean up config file
    std::remove(configFile.c_str());
    
    // Parse and return result
    try {
        return json::parse(result);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse simulation engine output: " + result);
    }
}

std::string SimulationEngineClient::createConfigFile(
    const std::string& dataPath,
    const std::string& strategyName,
    const std::map<std::string, double>& strategyParams,
    double initialCash) {
    
    // Create temporary file
    std::string tempTemplate = "/tmp/fingraph_config_XXXXXX";
    std::vector<char> tempFile(tempTemplate.begin(), tempTemplate.end());
    tempFile.push_back('\0');
    int fid = mkstemp(tempFile.data());
    if (fid == -1) {
        throw std::runtime_error("Failed to create temporary config file");
    }
    close(fid);
    
    // Write config to file
    json config;
    config["dataPath"] = dataPath;
    config["strategy"] = strategyName;
    config["parameters"] = strategyParams;
    config["initialCash"] = initialCash;
    
    std::string filename(tempFile.data());

    std::ofstream file(filename);
    file << config.dump();
    file.close();
    
    return filename;
}