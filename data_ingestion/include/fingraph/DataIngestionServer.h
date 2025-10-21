#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>

namespace fingraph {

struct OHLCV {
    std::string timestamp;
    double open;
    double high;
    double low;
    double close;
    int64_t volume;
};

struct DataUploadResponse {
    bool success;
    std::string message;
    std::string data_id;
    size_t rows_processed;
};

struct DataInfo {
    std::string id;
    std::string symbol;
    size_t rows;
    std::string date_range_start;
    std::string date_range_end;
    std::string last_modified;
};

struct DataPreview {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    size_t total_rows;
};

class DataIngestionServer {
public:
    DataIngestionServer(const std::string& database_path = "fingraph_data.db");
    ~DataIngestionServer();

    // Server lifecycle
    bool start(const std::string& server_address = "0.0.0.0:8081");
    void stop();
    void wait();

    // HTTP API endpoints
    DataUploadResponse uploadData(const std::string& symbol, const std::vector<OHLCV>& data);
    bool deleteData(const std::string& data_id);
    std::vector<DataInfo> listAvailableData();
    DataInfo getDataInfo(const std::string& data_id);
    DataPreview getDataPreview(const std::string& data_id, size_t limit = 100);
    std::vector<OHLCV> getData(const std::string& data_id, 
                              const std::string& start_date = "", 
                              const std::string& end_date = "");

    // External API integration
    bool fetchFromAlphaVantage(const std::string& symbol, const std::string& api_key);
    bool fetchFromYahooFinance(const std::string& symbol);

private:
    // HTTP server implementation
    void serverThread();
    void setupRoutes();
    
    // Request handlers
    std::string handleUploadData(const std::string& request_body);
    std::string handleDeleteData(const std::string& data_id);
    std::string handleListData();
    std::string handleGetDataInfo(const std::string& data_id);
    std::string handleDataPreview(const std::string& data_id, const std::string& query_params);
    std::string handleGetData(const std::string& data_id, const std::string& query_params);
    std::string handleFetchExternal(const std::string& request_body);
    std::string handleHealthCheck();

    // Data processing
    std::vector<OHLCV> parseCSVData(const std::string& csv_content);
    std::vector<OHLCV> parseJSONData(const std::string& json_content);
    bool validateOHLCVData(const std::vector<OHLCV>& data);
    std::string generateDataId(const std::string& symbol);
    
    // Database operations
    bool initializeDatabase();
    bool saveDataToDatabase(const std::string& data_id, const std::vector<OHLCV>& data);
    std::vector<OHLCV> loadDataFromDatabase(const std::string& data_id,
                                           const std::string& start_date = "",
                                           const std::string& end_date = "");
    
    // External API helpers
    std::vector<OHLCV> fetchAlphaVantageData(const std::string& symbol, const std::string& api_key);
    std::vector<OHLCV> fetchYahooFinanceData(const std::string& symbol);
    std::string makeHttpRequest(const std::string& url);

    // Utility methods
    std::string jsonResponse(bool success, const std::string& message, const nlohmann::json& data = {});
    std::string getCurrentTimestamp();
    std::string formatDateRange(const std::vector<OHLCV>& data);

private:
    std::string server_address_;
    std::string database_path_;
    std::unique_ptr<std::thread> server_thread_;
    std::atomic<bool> running_;
    
    // Database connection (simplified for now)
    struct DatabaseImpl;
    std::unique_ptr<DatabaseImpl> db_;
    
    // HTTP server implementation (simplified for now)
    void* http_server_; // Will be actual HTTP server implementation
};

} // namespace fingraph