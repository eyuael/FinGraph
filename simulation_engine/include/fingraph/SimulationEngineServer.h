#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "fingraph/JobManager.h"

namespace fingraph {

// Forward declarations for gRPC types (will be replaced with actual protobuf types)
class ServerContext;
class ServerWriter;
class Status;

// Placeholder structures for protobuf types (will be replaced by generated classes)
struct JobResponse {
    std::string job_id;
    int status;
    std::string message;
};

struct JobStatusRequest {
    std::string job_id;
};

struct JobResultsRequest {
    std::string job_id;
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
    // Will contain strategy information
};

struct StrategyParamsRequest {
    std::string strategy_name;
};

struct StrategyParamsResponse {
    // Will contain parameter information
};

struct JobProgressUpdate {
    std::string job_id;
    double progress;
    std::string current_step;
    std::string message;
};

class SimulationEngineServer {
public:
    SimulationEngineServer(size_t max_concurrent_jobs = 4);
    ~SimulationEngineServer();

    // Server lifecycle
    bool start(const std::string& server_address = "0.0.0.0:50051");
    void stop();
    void wait();

    // gRPC service implementations (these will be implemented as actual gRPC methods)
    // For now, we'll create wrapper methods that can be called from the gRPC service
    
    // Job management
    std::string submitBacktest(const BacktestRequest& request, JobResponse& response);
    bool getJobStatus(const JobStatusRequest& request, JobStatusResponse& response);
    bool getJobResults(const JobResultsRequest& request, BacktestResults& response);
    bool cancelJob(const CancelJobRequest& request, CancelJobResponse& response);
    
    // Strategy information
    bool listStrategies(const ListStrategiesRequest& request, ListStrategiesResponse& response);
    bool getStrategyParameters(const StrategyParamsRequest& request, StrategyParamsResponse& response);
    
    // Progress streaming (will be implemented with actual gRPC streaming)
    bool streamJobProgress(const JobStatusRequest& request, 
                          void* writer); // Will be ServerWriter<JobProgressUpdate>*

private:
    // Helper methods
    void initializeStrategies();
    void convertToProtobufJobStatus(const JobStatus& status, int& proto_status);
    void convertFromProtobufJobStatus(int proto_status, JobStatus& status);
    
    // Strategy information helpers
    void populateStrategyList(ListStrategiesResponse& response);
    void populateStrategyParameters(const std::string& strategy_name, 
                                   StrategyParamsResponse& response);

private:
    std::unique_ptr<JobManager> job_manager_;
    std::unique_ptr<std::thread> server_thread_;
    std::atomic<bool> running_;
    std::string server_address_;
    
    // Strategy registry (for listing available strategies)
    std::vector<std::string> available_strategies_;
    std::map<std::string, std::map<std::string, double>> strategy_parameters_;
};

// gRPC service implementation class (will be generated from protobuf)
class SimulationEngineServiceImpl {
public:
    explicit SimulationEngineServiceImpl(SimulationEngineServer* server);
    
    // These methods will be implemented as actual gRPC service methods
    // Status SubmitBacktest(ServerContext* context, const BacktestRequest* request, JobResponse* response) override;
    // Status GetJobStatus(ServerContext* context, const JobStatusRequest* request, JobStatusResponse* response) override;
    // ... etc

private:
    SimulationEngineServer* server_;
};

} // namespace fingraph