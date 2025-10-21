#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <vector>
#include "fingraph/Backtest.h"
#include "fingraph/Trade.h"

namespace fingraph {

enum class JobStatus {
    PENDING = 0,
    RUNNING = 1,
    COMPLETED = 2,
    FAILED = 3,
    CANCELLED = 4
};

struct BacktestRequest {
    std::string data_path;
    std::string strategy_name;
    std::map<std::string, double> strategy_params;
    double initial_cash;
    std::string job_id;
};

struct TradeData {
    std::string symbol;
    std::string type;
    double quantity;
    double price;
    int64_t timestamp;
};

struct EquityPoint {
    int64_t timestamp;
    double value;
};

struct BacktestResults {
    std::string job_id;
    double total_return;
    double sharpe_ratio;
    double max_drawdown;
    double win_rate;
    std::vector<TradeData> trades;
    std::vector<EquityPoint> equity_curve;
};

struct JobStatusResponse {
    std::string job_id;
    JobStatus status;
    double progress;
    std::string message;
    int64_t start_time;
    int64_t estimated_completion;
};

struct Job {
    std::string id;
    JobStatus status;
    BacktestRequest request;
    BacktestResults result;
    std::string error_message;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point completed_at;
    double progress;
    std::string current_step;
    
    Job() : status(JobStatus::PENDING), progress(0.0) {
        created_at = std::chrono::system_clock::now();
    }
};

using JobPtr = std::shared_ptr<Job>;
using ProgressCallback = std::function<void(const std::string&, double, const std::string&)>;

class JobManager {
public:
    JobManager(size_t max_concurrent_jobs = 4);
    ~JobManager();

    // Job submission and management
    std::string submitJob(const BacktestRequest& request);
    bool cancelJob(const std::string& job_id);
    JobPtr getJob(const std::string& job_id);
    
    // Job status and results
    JobStatusResponse getJobStatus(const std::string& job_id);
    BacktestResults getJobResults(const std::string& job_id);
    
    // Progress tracking
    void setProgressCallback(ProgressCallback callback);
    void updateJobProgress(const std::string& job_id, double progress, const std::string& step);
    
    // Job queue management
    void start();
    void stop();
    size_t getQueueSize() const;
    size_t getRunningJobsCount() const;
    
    // Cleanup
    void cleanupCompletedJobs(std::chrono::hours max_age = std::chrono::hours(24));

private:
    // Worker thread function
    void workerThread();
    
    // Job execution
    void executeJob(JobPtr job);
    BacktestResults runBacktest(const BacktestRequest& request, JobPtr job);
    
    // Internal job management
    void markJobRunning(JobPtr job);
    void markJobCompleted(JobPtr job, const BacktestResults& result);
    void markJobFailed(JobPtr job, const std::string& error);
    
    // Thread-safe operations
    void pushJobToQueue(JobPtr job);
    JobPtr popJobFromQueue();
    bool hasJobsInQueue() const;

private:
    mutable std::mutex jobs_mutex_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::condition_variable jobs_cv_;
    
    std::unordered_map<std::string, JobPtr> jobs_;
    std::queue<JobPtr> job_queue_;
    
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_;
    size_t max_concurrent_jobs_;
    std::atomic<size_t> running_jobs_count_;
    
    ProgressCallback progress_callback_;
    
    // Job ID generation
    std::atomic<uint64_t> job_counter_;
    std::string generateJobId();
};

} // namespace fingraph