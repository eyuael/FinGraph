#include "fingraph/JobManager.h"
#include <sstream>
#include <iomanip>
#include <random>

namespace fingraph {

JobManager::JobManager(size_t max_concurrent_jobs)
    : running_(false)
    , max_concurrent_jobs_(max_concurrent_jobs)
    , running_jobs_count_(0)
    , job_counter_(0) {
}

JobManager::~JobManager() {
    stop();
}

std::string JobManager::submitJob(const BacktestRequest& request) {
    auto job = std::make_shared<Job>();
    job->id = generateJobId();
    job->request = request;
    job->request.job_id = job->id;
    
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        jobs_[job->id] = job;
    }
    
    pushJobToQueue(job);
    
    return job->id;
}

bool JobManager::cancelJob(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return false;
    }
    
    JobPtr job = it->second;
    if (job->status == JobStatus::PENDING) {
        job->status = JobStatus::CANCELLED;
        job->completed_at = std::chrono::system_clock::now();
        return true;
    }
    
    return false;
}

JobPtr JobManager::getJob(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    return (it != jobs_.end()) ? it->second : nullptr;
}

JobStatusResponse JobManager::getJobStatus(const std::string& job_id) {
    JobStatusResponse response;
    response.job_id = job_id;
    response.progress = 0.0;
    response.start_time = 0;
    response.estimated_completion = 0;
    
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        response.status = JobStatus::FAILED;
        response.message = "Job not found";
        return response;
    }
    
    JobPtr job = it->second;
    response.status = job->status;
    response.progress = job->progress;
    response.message = job->current_step;
    
    if (job->started_at != std::chrono::system_clock::time_point{}) {
        response.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            job->started_at.time_since_epoch()).count();
    }
    
    return response;
}

BacktestResults JobManager::getJobResults(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    if (it == jobs_.end() || it->second->status != JobStatus::COMPLETED) {
        return BacktestResults{};
    }
    
    return it->second->result;
}

void JobManager::setProgressCallback(ProgressCallback callback) {
    progress_callback_ = std::move(callback);
}

void JobManager::updateJobProgress(const std::string& job_id, double progress, const std::string& step) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    if (it != jobs_.end()) {
        it->second->progress = progress;
        it->second->current_step = step;
        
        if (progress_callback_) {
            progress_callback_(job_id, progress, step);
        }
    }
}

void JobManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // Start worker threads
    for (size_t i = 0; i < max_concurrent_jobs_; ++i) {
        worker_threads_.emplace_back(&JobManager::workerThread, this);
    }
}

void JobManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    queue_cv_.notify_all();
    
    // Wait for all worker threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
}

size_t JobManager::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return job_queue_.size();
}

size_t JobManager::getRunningJobsCount() const {
    return running_jobs_count_.load();
}

void JobManager::cleanupCompletedJobs(std::chrono::hours max_age) {
    auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.begin();
    while (it != jobs_.end()) {
        JobPtr job = it->second;
        if ((job->status == JobStatus::COMPLETED || job->status == JobStatus::FAILED) &&
            job->completed_at != std::chrono::system_clock::time_point{} &&
            (now - job->completed_at) > max_age) {
            it = jobs_.erase(it);
        } else {
            ++it;
        }
    }
}

void JobManager::workerThread() {
    while (running_) {
        JobPtr job = popJobFromQueue();
        if (!job) {
            // No job available, wait for one
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !job_queue_.empty() || !running_; });
            continue;
        }
        
        executeJob(job);
        running_jobs_count_--;
    }
}

void JobManager::executeJob(JobPtr job) {
    markJobRunning(job);
    
    try {
        BacktestResults result = runBacktest(job->request, job);
        markJobCompleted(job, result);
    } catch (const std::exception& e) {
        markJobFailed(job, e.what());
    }
}

BacktestResults JobManager::runBacktest(const BacktestRequest& request, JobPtr job) {
    BacktestResults results;
    results.job_id = request.job_id;
    
    updateJobProgress(job->id, 0.1, "Initializing backtest engine");
    
    // Create and configure the backtest engine
    BacktestEngine engine;
    
    updateJobProgress(job->id, 0.2, "Loading market data");
    
    // Run the backtest
    BacktestResult engine_result = engine.runBacktest(
        request.data_path,
        request.strategy_name,
        request.strategy_params,
        request.initial_cash
    );
    
    updateJobProgress(job->id, 0.8, "Processing results");
    
    // Convert engine result to our format
    results.total_return = engine_result.totalReturn;
    results.sharpe_ratio = engine_result.sharpeRatio;
    results.max_drawdown = engine_result.maxDrawdown;
    results.win_rate = engine_result.winRate;
    
    // Convert trades
    for (const auto& trade : engine_result.trades) {
        TradeData trade_data;
        trade_data.symbol = trade.getSymbol();
        trade_data.type = (trade.getType() == TradeType::BUY) ? "BUY" : "SELL";
        trade_data.quantity = trade.getQuantity();
        trade_data.price = trade.getPrice();
        trade_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            trade.getTimestamp().time_since_epoch()).count();
        results.trades.push_back(trade_data);
    }
    
    // Convert equity curve
    for (const auto& point : engine_result.equityCurve) {
        EquityPoint equity_point;
        equity_point.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            point.first.time_since_epoch()).count();
        equity_point.value = point.second;
        results.equity_curve.push_back(equity_point);
    }
    
    updateJobProgress(job->id, 1.0, "Backtest completed");
    
    return results;
}

void JobManager::markJobRunning(JobPtr job) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    job->status = JobStatus::RUNNING;
    job->started_at = std::chrono::system_clock::now();
    job->current_step = "Starting execution";
    running_jobs_count_++;
}

void JobManager::markJobCompleted(JobPtr job, const BacktestResults& result) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    job->status = JobStatus::COMPLETED;
    job->result = result;
    job->completed_at = std::chrono::system_clock::now();
    job->progress = 1.0;
    job->current_step = "Completed";
}

void JobManager::markJobFailed(JobPtr job, const std::string& error) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    job->status = JobStatus::FAILED;
    job->error_message = error;
    job->completed_at = std::chrono::system_clock::now();
    job->current_step = "Failed: " + error;
}

void JobManager::pushJobToQueue(JobPtr job) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    job_queue_.push(job);
    queue_cv_.notify_one();
}

JobPtr JobManager::popJobFromQueue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (job_queue_.empty()) {
        return nullptr;
    }
    
    JobPtr job = job_queue_.front();
    job_queue_.pop();
    return job;
}

bool JobManager::hasJobsInQueue() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !job_queue_.empty();
}

std::string JobManager::generateJobId() {
    uint64_t counter = job_counter_++;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::stringstream ss;
    ss << "job_" << timestamp << "_" << counter << "_" << dis(gen);
    return ss.str();
}

} // namespace fingraph