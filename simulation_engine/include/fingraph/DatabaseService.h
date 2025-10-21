#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fingraph {

struct JobRecord {
    std::string id;
    std::string status;
    json request_data;
    json result_data;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point completed_at;
    std::string error_message;
};

struct MarketDataRecord {
    std::string symbol;
    std::chrono::system_clock::time_point timestamp;
    double open_price;
    double high_price;
    double low_price;
    double close_price;
    int64_t volume;
};

class DatabaseService {
public:
    DatabaseService(const std::string& connection_string);
    ~DatabaseService();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Job management
    bool saveJob(const JobRecord& job);
    std::unique_ptr<JobRecord> getJob(const std::string& job_id);
    std::vector<JobRecord> getJobsByStatus(const std::string& status);
    std::vector<JobRecord> getRecentJobs(size_t limit = 100);
    bool updateJobStatus(const std::string& job_id, const std::string& status);
    bool updateJobResult(const std::string& job_id, const json& result);
    bool deleteJob(const std::string& job_id);
    size_t cleanupOldJobs(std::chrono::hours max_age = std::chrono::hours(24));

    // Market data management
    bool saveMarketData(const std::vector<MarketDataRecord>& records);
    std::vector<MarketDataRecord> getMarketData(
        const std::string& symbol,
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time
    );
    std::vector<std::string> getAvailableSymbols();
    bool deleteMarketData(const std::string& symbol, 
                         const std::chrono::system_clock::time_point& before_time = {});

    // Schema management
    bool initializeSchema();
    bool validateSchema();

private:
    // SQL query helpers
    bool executeQuery(const std::string& query);
    bool executeQuery(const std::string& query, const std::vector<std::string>& params);
    
    // Conversion helpers
    std::string timePointToString(const std::chrono::system_clock::time_point& tp) const;
    std::chrono::system_clock::time_point stringToTimePoint(const std::string& str) const;
    std::string jsonToString(const json& j) const;
    json stringToJson(const std::string& str) const;

    // Database-specific implementations
    struct DatabaseImpl;
    std::unique_ptr<DatabaseImpl> pimpl_;

    std::string connection_string_;
    bool connected_;
};

} // namespace fingraph