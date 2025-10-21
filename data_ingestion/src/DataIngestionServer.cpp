#include "fingraph/DataIngestionServer.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <curl/curl.h>
#include <sqlite3.h>

using json = nlohmann::json;

namespace fingraph {

// Private implementation for database
struct DataIngestionServer::DatabaseImpl {
    sqlite3* db = nullptr;
    
    ~DatabaseImpl() {
        if (db) {
            sqlite3_close(db);
        }
    }
};

// HTTP callback for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

DataIngestionServer::DataIngestionServer(const std::string& database_path)
    : database_path_(database_path)
    , running_(false)
    , http_server_(nullptr)
    , db_(std::make_unique<DatabaseImpl>()) {
}

DataIngestionServer::~DataIngestionServer() {
    stop();
}

bool DataIngestionServer::start(const std::string& server_address) {
    if (running_) {
        return false;
    }
    
    server_address_ = server_address;
    
    // Initialize database
    if (!initializeDatabase()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return false;
    }
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    running_ = true;
    
    // Start server thread
    server_thread_ = std::make_unique<std::thread>(&DataIngestionServer::serverThread, this);
    
    std::cout << "Data Ingestion Server started on " << server_address_ << std::endl;
    return true;
}

void DataIngestionServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
    
    curl_global_cleanup();
    std::cout << "Data Ingestion Server stopped" << std::endl;
}

void DataIngestionServer::wait() {
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
}

bool DataIngestionServer::initializeDatabase() {
    int rc = sqlite3_open(database_path_.c_str(), &db_->db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_->db) << std::endl;
        return false;
    }
    
    // Create tables
    const char* create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS market_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            data_id TEXT NOT NULL,
            symbol TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            open_price REAL NOT NULL,
            high_price REAL NOT NULL,
            low_price REAL NOT NULL,
            close_price REAL NOT NULL,
            volume INTEGER NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            UNIQUE(data_id, symbol, timestamp)
        );
        
        CREATE TABLE IF NOT EXISTS data_info (
            data_id TEXT PRIMARY KEY,
            symbol TEXT NOT NULL,
            rows INTEGER NOT NULL,
            date_range_start TEXT,
            date_range_end TEXT,
            last_modified TEXT DEFAULT CURRENT_TIMESTAMP
        );
        
        CREATE INDEX IF NOT EXISTS idx_market_data_data_id ON market_data(data_id);
        CREATE INDEX IF NOT EXISTS idx_market_data_symbol ON market_data(symbol);
        CREATE INDEX IF NOT EXISTS idx_market_data_timestamp ON market_data(timestamp);
    )";
    
    char* error_msg = nullptr;
    rc = sqlite3_exec(db_->db, create_table_sql, nullptr, nullptr, &error_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << error_msg << std::endl;
        sqlite3_free(error_msg);
        return false;
    }
    
    return true;
}

void DataIngestionServer::serverThread() {
    // Simplified HTTP server implementation
    // In production, this would use a proper HTTP library like cpp-httplib or Crow
    
    std::cout << "HTTP server thread running (simplified implementation)" << std::endl;
    
    while (running_) {
        // Simulate HTTP server processing
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // In a real implementation, this would:
        // 1. Listen for HTTP connections
        // 2. Parse HTTP requests
        // 3. Route to appropriate handlers
        // 4. Send HTTP responses
    }
}

DataUploadResponse DataIngestionServer::uploadData(const std::string& symbol, const std::vector<OHLCV>& data) {
    DataUploadResponse response;
    
    if (data.empty()) {
        response.success = false;
        response.message = "No data provided";
        return response;
    }
    
    if (!validateOHLCVData(data)) {
        response.success = false;
        response.message = "Invalid data format";
        return response;
    }
    
    std::string data_id = generateDataId(symbol);
    
    if (saveDataToDatabase(data_id, data)) {
        response.success = true;
        response.message = "Data uploaded successfully";
        response.data_id = data_id;
        response.rows_processed = data.size();
    } else {
        response.success = false;
        response.message = "Failed to save data to database";
    }
    
    return response;
}

bool DataIngestionServer::deleteData(const std::string& data_id) {
    const char* sql = "DELETE FROM market_data WHERE data_id = ?; DELETE FROM data_info WHERE data_id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, data_id.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<DataInfo> DataIngestionServer::listAvailableData() {
    const char* sql = "SELECT data_id, symbol, rows, date_range_start, date_range_end, last_modified FROM data_info ORDER BY last_modified DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    std::vector<DataInfo> data_list;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        DataInfo info;
        info.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        info.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        info.rows = static_cast<size_t>(sqlite3_column_int(stmt, 2));
        info.date_range_start = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        info.date_range_end = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        info.last_modified = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        data_list.push_back(info);
    }
    
    sqlite3_finalize(stmt);
    return data_list;
}

DataInfo DataIngestionServer::getDataInfo(const std::string& data_id) {
    const char* sql = "SELECT data_id, symbol, rows, date_range_start, date_range_end, last_modified FROM data_info WHERE data_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
    
    DataInfo info;
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        info.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        info.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        info.rows = static_cast<size_t>(sqlite3_column_int(stmt, 2));
        info.date_range_start = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        info.date_range_end = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        info.last_modified = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }
    
    sqlite3_finalize(stmt);
    return info;
}

DataPreview DataIngestionServer::getDataPreview(const std::string& data_id, size_t limit) {
    DataPreview preview;
    preview.headers = {"timestamp", "open", "high", "low", "close", "volume"};
    
    const char* sql = "SELECT timestamp, open_price, high_price, low_price, close_price, volume FROM market_data WHERE data_id = ? ORDER BY timestamp LIMIT ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return preview;
    }
    
    sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(limit));
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row;
        row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        row.push_back(std::to_string(sqlite3_column_double(stmt, 1)));
        row.push_back(std::to_string(sqlite3_column_double(stmt, 2)));
        row.push_back(std::to_string(sqlite3_column_double(stmt, 3)));
        row.push_back(std::to_string(sqlite3_column_double(stmt, 4)));
        row.push_back(std::to_string(sqlite3_column_int64(stmt, 5)));
        preview.rows.push_back(row);
    }
    
    // Get total count
    sqlite3_finalize(stmt);
    const char* count_sql = "SELECT COUNT(*) FROM market_data WHERE data_id = ?";
    rc = sqlite3_prepare_v2(db_->db, count_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            preview.total_rows = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
        }
    }
    
    sqlite3_finalize(stmt);
    return preview;
}

std::vector<OHLCV> DataIngestionServer::getData(const std::string& data_id, 
                                                const std::string& start_date, 
                                                const std::string& end_date) {
    return loadDataFromDatabase(data_id, start_date, end_date);
}

bool DataIngestionServer::fetchFromAlphaVantage(const std::string& symbol, const std::string& api_key) {
    std::vector<OHLCV> data = fetchAlphaVantageData(symbol, api_key);
    if (data.empty()) {
        return false;
    }
    
    DataUploadResponse response = uploadData(symbol, data);
    return response.success;
}

bool DataIngestionServer::fetchFromYahooFinance(const std::string& symbol) {
    std::vector<OHLCV> data = fetchYahooFinanceData(symbol);
    if (data.empty()) {
        return false;
    }
    
    DataUploadResponse response = uploadData(symbol, data);
    return response.success;
}

std::vector<OHLCV> DataIngestionServer::parseCSVData(const std::string& csv_content) {
    std::vector<OHLCV> data;
    std::istringstream stream(csv_content);
    std::string line;
    
    // Skip header line
    if (!std::getline(stream, line)) {
        return data;
    }
    
    while (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        std::string field;
        std::vector<std::string> fields;
        
        while (std::getline(line_stream, field, ',')) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 6) {
            OHLCV ohlcv;
            ohlcv.timestamp = fields[0];
            ohlcv.open = std::stod(fields[1]);
            ohlcv.high = std::stod(fields[2]);
            ohlcv.low = std::stod(fields[3]);
            ohlcv.close = std::stod(fields[4]);
            ohlcv.volume = std::stoll(fields[5]);
            data.push_back(ohlcv);
        }
    }
    
    return data;
}

bool DataIngestionServer::validateOHLCVData(const std::vector<OHLCV>& data) {
    if (data.empty()) {
        return false;
    }
    
    for (const auto& ohlcv : data) {
        if (ohlcv.timestamp.empty() || 
            ohlcv.open <= 0 || ohlcv.high <= 0 || ohlcv.low <= 0 || ohlcv.close <= 0 ||
            ohlcv.volume < 0 ||
            ohlcv.high < ohlcv.low ||
            ohlcv.high < ohlcv.open || ohlcv.high < ohlcv.close ||
            ohlcv.low > ohlcv.open || ohlcv.low > ohlcv.close) {
            return false;
        }
    }
    
    return true;
}

std::string DataIngestionServer::generateDataId(const std::string& symbol) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return symbol + "_" + std::to_string(timestamp);
}

bool DataIngestionServer::saveDataToDatabase(const std::string& data_id, const std::vector<OHLCV>& data) {
    // Begin transaction
    sqlite3_exec(db_->db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    
    const char* sql = "INSERT OR REPLACE INTO market_data (data_id, symbol, timestamp, open_price, high_price, low_price, close_price, volume) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db_->db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    
    bool success = true;
    for (const auto& ohlcv : data) {
        sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, data_id.substr(0, data_id.find_last_of('_')).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, ohlcv.timestamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, ohlcv.open);
        sqlite3_bind_double(stmt, 5, ohlcv.high);
        sqlite3_bind_double(stmt, 6, ohlcv.low);
        sqlite3_bind_double(stmt, 7, ohlcv.close);
        sqlite3_bind_int64(stmt, 8, ohlcv.volume);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            success = false;
            break;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    
    if (success) {
        // Update data_info table
        std::string date_range = formatDateRange(data);
        const char* info_sql = "INSERT OR REPLACE INTO data_info (data_id, symbol, rows, date_range_start, date_range_end) VALUES (?, ?, ?, ?, ?)";
        
        rc = sqlite3_prepare_v2(db_->db, info_sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            std::string symbol = data_id.substr(0, data_id.find_last_of('_'));
            size_t pos = date_range.find(" - ");
            std::string start_date = (pos != std::string::npos) ? date_range.substr(0, pos) : "";
            std::string end_date = (pos != std::string::npos) ? date_range.substr(pos + 3) : "";
            
            sqlite3_bind_text(stmt, 1, data_id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, symbol.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 3, static_cast<int64_t>(data.size()));
            sqlite3_bind_text(stmt, 4, start_date.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, end_date.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            
            if (rc == SQLITE_DONE) {
                sqlite3_exec(db_->db, "COMMIT", nullptr, nullptr, nullptr);
            } else {
                success = false;
            }
        } else {
            success = false;
        }
    }
    
    if (!success) {
        sqlite3_exec(db_->db, "ROLLBACK", nullptr, nullptr, nullptr);
    }
    
    return success;
}

std::vector<OHLCV> DataIngestionServer::loadDataFromDatabase(const std::string& data_id,
                                                           const std::string& start_date,
                                                           const std::string& end_date) {
    std::vector<OHLCV> data;
    
    std::string sql = "SELECT timestamp, open_price, high_price, low_price, close_price, volume FROM market_data WHERE data_id = ?";
    if (!start_date.empty()) {
        sql += " AND timestamp >= ?";
    }
    if (!end_date.empty()) {
        sql += " AND timestamp <= ?";
    }
    sql += " ORDER BY timestamp";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_->db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return data;
    }
    
    int param_index = 1;
    sqlite3_bind_text(stmt, param_index++, data_id.c_str(), -1, SQLITE_STATIC);
    if (!start_date.empty()) {
        sqlite3_bind_text(stmt, param_index++, start_date.c_str(), -1, SQLITE_STATIC);
    }
    if (!end_date.empty()) {
        sqlite3_bind_text(stmt, param_index++, end_date.c_str(), -1, SQLITE_STATIC);
    }
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        OHLCV ohlcv;
        ohlcv.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        ohlcv.open = sqlite3_column_double(stmt, 1);
        ohlcv.high = sqlite3_column_double(stmt, 2);
        ohlcv.low = sqlite3_column_double(stmt, 3);
        ohlcv.close = sqlite3_column_double(stmt, 4);
        ohlcv.volume = sqlite3_column_int64(stmt, 5);
        data.push_back(ohlcv);
    }
    
    sqlite3_finalize(stmt);
    return data;
}

std::vector<OHLCV> DataIngestionServer::fetchAlphaVantageData(const std::string& symbol, const std::string& api_key) {
    // Placeholder implementation
    std::cout << "Fetching data from Alpha Vantage for " << symbol << std::endl;
    // In a real implementation, this would make HTTP requests to Alpha Vantage API
    return {};
}

std::vector<OHLCV> DataIngestionServer::fetchYahooFinanceData(const std::string& symbol) {
    // Placeholder implementation
    std::cout << "Fetching data from Yahoo Finance for " << symbol << std::endl;
    // In a real implementation, this would make HTTP requests to Yahoo Finance API
    return {};
}

std::string DataIngestionServer::makeHttpRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string response;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

std::string DataIngestionServer::jsonResponse(bool success, const std::string& message, const json& data) {
    json response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = data;
    }
    return response.dump();
}

std::string DataIngestionServer::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string DataIngestionServer::formatDateRange(const std::vector<OHLCV>& data) {
    if (data.empty()) {
        return "";
    }
    
    return data.front().timestamp + " - " + data.back().timestamp;
}

} // namespace fingraph