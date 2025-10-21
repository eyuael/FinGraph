#include "fingraph/SimulationEngineServer.h"
#include "fingraph/Backtest.h"
#include <iostream>
#include <chrono>

namespace fingraph {

SimulationEngineServer::SimulationEngineServer(size_t max_concurrent_jobs)
    : running_(false) {
    job_manager_ = std::make_unique<JobManager>(max_concurrent_jobs);
    initializeStrategies();
}

SimulationEngineServer::~SimulationEngineServer() {
    stop();
}

bool SimulationEngineServer::start(const std::string& server_address) {
    if (running_) {
        return false;
    }
    
    server_address_ = server_address;
    running_ = true;
    
    // Start the job manager
    job_manager_->start();
    
    // TODO: Start actual gRPC server here
    // For now, we'll just start a thread that simulates the server
    server_thread_ = std::make_unique<std::thread>([this]() {
        std::cout << "Simulation Engine gRPC Server started on " << server_address_ << std::endl;
        
        // Simulate server running
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        std::cout << "Simulation Engine gRPC Server stopped" << std::endl;
    });
    
    return true;
}

void SimulationEngineServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (job_manager_) {
        job_manager_->stop();
    }
    
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
}

void SimulationEngineServer::wait() {
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
}

std::string SimulationEngineServer::submitBacktest(const BacktestRequest& request, JobResponse& response) {
    std::string job_id = job_manager_->submitJob(request);
    
    response.job_id = job_id;
    response.status = 0; // PENDING
    response.message = "Job submitted successfully";
    
    return job_id;
}

bool SimulationEngineServer::getJobStatus(const JobStatusRequest& request, JobStatusResponse& response) {
    response = job_manager_->getJobStatus(request.job_id);
    return !response.job_id.empty();
}

bool SimulationEngineServer::getJobResults(const JobResultsRequest& request, BacktestResults& response) {
    response = job_manager_->getJobResults(request.job_id);
    return !response.job_id.empty();
}

bool SimulationEngineServer::cancelJob(const CancelJobRequest& request, CancelJobResponse& response) {
    bool success = job_manager_->cancelJob(request.job_id);
    response.success = success;
    response.message = success ? "Job cancelled successfully" : "Failed to cancel job";
    return success;
}

bool SimulationEngineServer::listStrategies(const ListStrategiesRequest& request, ListStrategiesResponse& response) {
    populateStrategyList(response);
    return true;
}

bool SimulationEngineServer::getStrategyParameters(const StrategyParamsRequest& request, StrategyParamsResponse& response) {
    populateStrategyParameters(request.strategy_name, response);
    return !response.parameters.empty();
}

bool SimulationEngineServer::streamJobProgress(const JobStatusRequest& request, void* writer) {
    // TODO: Implement actual gRPC streaming
    // For now, this is a placeholder
    std::cout << "Streaming progress for job: " << request.job_id << std::endl;
    return true;
}

void SimulationEngineServer::initializeStrategies() {
    // Initialize available strategies
    available_strategies_ = {"MovingAverage", "RSI"};
    
    // Initialize strategy parameters
    strategy_parameters_["MovingAverage"] = {
        {"short_window", 10},
        {"long_window", 20}
    };
    
    strategy_parameters_["RSI"] = {
        {"period", 14},
        {"overbought_threshold", 70},
        {"oversold_threshold", 30}
    };
}

void SimulationEngineServer::convertToProtobufJobStatus(const JobStatus& status, int& proto_status) {
    switch (status) {
        case JobStatus::PENDING:
            proto_status = 0;
            break;
        case JobStatus::RUNNING:
            proto_status = 1;
            break;
        case JobStatus::COMPLETED:
            proto_status = 2;
            break;
        case JobStatus::FAILED:
            proto_status = 3;
            break;
        case JobStatus::CANCELLED:
            proto_status = 4;
            break;
    }
}

void SimulationEngineServer::convertFromProtobufJobStatus(int proto_status, JobStatus& status) {
    switch (proto_status) {
        case 0:
            status = JobStatus::PENDING;
            break;
        case 1:
            status = JobStatus::RUNNING;
            break;
        case 2:
            status = JobStatus::COMPLETED;
            break;
        case 3:
            status = JobStatus::FAILED;
            break;
        case 4:
            status = JobStatus::CANCELLED;
            break;
        default:
            status = JobStatus::FAILED;
            break;
    }
}

void SimulationEngineServer::populateStrategyList(ListStrategiesResponse& response) {
    // TODO: This will populate the actual protobuf response structure
    // For now, we'll just log the available strategies
    std::cout << "Available strategies:" << std::endl;
    for (const auto& strategy : available_strategies_) {
        std::cout << " - " << strategy << std::endl;
    }
}

void SimulationEngineServer::populateStrategyParameters(const std::string& strategy_name, 
                                                        StrategyParamsResponse& response) {
    // TODO: This will populate the actual protobuf response structure
    // For now, we'll just log the parameters
    auto it = strategy_parameters_.find(strategy_name);
    if (it != strategy_parameters_.end()) {
        std::cout << "Parameters for " << strategy_name << ":" << std::endl;
        for (const auto& param : it->second) {
            std::cout << " - " << param.first << ": " << param.second << std::endl;
        }
    }
}

SimulationEngineServiceImpl::SimulationEngineServiceImpl(SimulationEngineServer* server)
    : server_(server) {
}

} // namespace fingraph