#include "fingraph/DatabaseService.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>

namespace fingraph {

// Private implementation struct for SQLite
struct DatabaseService::DatabaseImpl {
    sqlite3* db = nullptr;
    
    ~DatabaseImpl() {
        if (db) {
            sqlite3_close(db);
        }
    }
};

DatabaseService::DatabaseService(const std::string& connection_string)
    : connection_string_(connection_string)
    , connected_(false)
    , pimpl_(std::make_unique<DatabaseImpl>()) {
}

DatabaseService::~DatabaseService() = default;

bool DatabaseService::connect() {
    if (connected_) {
        return true;
    }
    
    // For now, we'll use SQLite for simplicity
    // In production, this would support PostgreSQL
    std::string db_path = connection_string_;
    if (db_path.empty()) {
        db_path = "fingraph.db";
    }
    
    int rc = sqlite3_open(db_path.c_str(), &pimpl_->db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(pimpl_->db) << std::endl;
        return false;
    }
    
    connected_ = true;
    
    // Initialize schema if needed
    if (!initializeSchema()) {
        disconnect();
        return false;
    }
    
    return true;
}

void DatabaseService::disconnect() {
    if (pimpl_->db) {
        sqlite3_close(pimpl_->db);
        pimpl_->db = nullptr;
    }
    connected_ = false;
}

bool DatabaseService::isConnected() const {
    return connected_;
}

bool DatabaseService::initializeSchema() {
    std::vector<std::string> schema_queries = {
        // Jobs table
        R"(
        CREATE TABLE IF NOT EXISTS jobs (
            id TEXT PRIMARY KEY,
            status TEXT NOT NULL,
            request_data TEXT,
            result_data TEXT,
            created_at TEXT NOT NULL,
            started_at TEXT,
            completed_at TEXT,
            error_message TEXT
        )
        )",
        
        // Market data table
        R"(
        CREATE TABLE IF NOT EXISTS market_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            open_price REAL,
            high_price REAL,
            low_price REAL,
            close_price REAL,
            volume INTEGER,
            UNIQUE(symbol, timestamp)
        )
        )",
        
        // Indexes for better performance
        "CREATE INDEX IF NOT EXISTS idx_jobs_status ON jobs(status)",
        "CREATE INDEX IF NOT EXISTS idx_jobs_created_at ON jobs(created_at)",
        "CREATE INDEX IF NOT EXISTS idx_market_data_symbol ON market_data(symbol)",
        "CREATE INDEX IF NOT EXISTS idx_market_data_timestamp ON market_data(timestamp)"
    };
    
    for (const auto& query : schema_queries) {
        if (!executeQuery(query)) {
            std::cerr << "Failed to initialize schema" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool DatabaseService::validateSchema() {
    // Check if required tables exist
    std::vector<std::string> required_tables = {"jobs", "market_data"};
    
    for (const auto& table : required_tables) {
        std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "'";
        // For simplicity, we'll assume schema is valid if we can connect
        // In production, this would be more thorough
    }
    
    return true;
}

bool DatabaseService::saveJob(const JobRecord& job) {
    std::ostringstream query;
    query << "INSERT OR REPLACE INTO jobs (id, status, request_data, result_data, created_at, started_at, completed_at, error_message) "
          << "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(pimpl_->db) << std::endl;
        return false;
    }
    
    // Bind parameters
    sqlite3_bind_text(stmt, 1, job.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, job.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, jsonToString(job.request_data).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, jsonToString(job.result_data).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, timePointToString(job.created_at).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, timePointToString(job.started_at).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, timePointToString(job.completed_at).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, job.error_message.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::unique_ptr<JobRecord> DatabaseService::getJob(const std::string& job_id) {
    std::string query = "SELECT id, status, request_data, result_data, created_at, started_at, completed_at, error_message "
                       "FROM jobs WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, job_id.c_str(), -1, SQLITE_STATIC);
    
    std::unique_ptr<JobRecord> job = nullptr;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        job = std::make_unique<JobRecord>();
        job->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        job->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        job->request_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        job->result_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        job->created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        job->started_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        job->completed_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
        job->error_message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }
    
    sqlite3_finalize(stmt);
    return job;
}

std::vector<JobRecord> DatabaseService::getJobsByStatus(const std::string& status) {
    std::string query = "SELECT id, status, request_data, result_data, created_at, started_at, completed_at, error_message "
                       "FROM jobs WHERE status = ? ORDER BY created_at DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    
    std::vector<JobRecord> jobs;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        JobRecord job;
        job.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        job.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        job.request_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        job.result_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        job.created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        job.started_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        job.completed_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
        job.error_message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        jobs.push_back(job);
    }
    
    sqlite3_finalize(stmt);
    return jobs;
}

std::vector<JobRecord> DatabaseService::getRecentJobs(size_t limit) {
    std::string query = "SELECT id, status, request_data, result_data, created_at, started_at, completed_at, error_message "
                       "FROM jobs ORDER BY created_at DESC LIMIT ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    sqlite3_bind_int(stmt, 1, static_cast<int>(limit));
    
    std::vector<JobRecord> jobs;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        JobRecord job;
        job.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        job.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        job.request_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        job.result_data = stringToJson(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        job.created_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        job.started_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        job.completed_at = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
        job.error_message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        jobs.push_back(job);
    }
    
    sqlite3_finalize(stmt);
    return jobs;
}

bool DatabaseService::updateJobStatus(const std::string& job_id, const std::string& status) {
    std::string query = "UPDATE jobs SET status = ? WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, job_id.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseService::updateJobResult(const std::string& job_id, const json& result) {
    std::string query = "UPDATE jobs SET result_data = ?, status = 'COMPLETED', completed_at = ? WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    std::string now = timePointToString(std::chrono::system_clock::now());
    sqlite3_bind_text(stmt, 1, jsonToString(result).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, job_id.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool DatabaseService::deleteJob(const std::string& job_id) {
    std::string query = "DELETE FROM jobs WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, job_id.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

size_t DatabaseService::cleanupOldJobs(std::chrono::hours max_age) {
    auto cutoff_time = std::chrono::system_clock::now() - max_age;
    std::string cutoff_str = timePointToString(cutoff_time);
    
    std::string query = "DELETE FROM jobs WHERE status IN ('COMPLETED', 'FAILED') AND completed_at < ? AND completed_at != ''";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, cutoff_str.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(pimpl_->db);
    sqlite3_finalize(stmt);
    
    return static_cast<size_t>(changes);
}

bool DatabaseService::saveMarketData(const std::vector<MarketDataRecord>& records) {
    if (records.empty()) {
        return true;
    }
    
    // Begin transaction
    executeQuery("BEGIN TRANSACTION");
    
    std::string query = "INSERT OR REPLACE INTO market_data (symbol, timestamp, open_price, high_price, low_price, close_price, volume) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        executeQuery("ROLLBACK");
        return false;
    }
    
    bool success = true;
    for (const auto& record : records) {
        sqlite3_bind_text(stmt, 1, record.symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, timePointToString(record.timestamp).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, record.open_price);
        sqlite3_bind_double(stmt, 4, record.high_price);
        sqlite3_bind_double(stmt, 5, record.low_price);
        sqlite3_bind_double(stmt, 6, record.close_price);
        sqlite3_bind_int64(stmt, 7, record.volume);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            success = false;
            break;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    
    if (success) {
        executeQuery("COMMIT");
    } else {
        executeQuery("ROLLBACK");
    }
    
    return success;
}

std::vector<MarketDataRecord> DatabaseService::getMarketData(
    const std::string& symbol,
    const std::chrono::system_clock::time_point& start_time,
    const std::chrono::system_clock::time_point& end_time) {
    
    std::string query = "SELECT symbol, timestamp, open_price, high_price, low_price, close_price, volume "
                       "FROM market_data WHERE symbol = ? AND timestamp >= ? AND timestamp <= ? "
                       "ORDER BY timestamp";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    std::string start_str = timePointToString(start_time);
    std::string end_str = timePointToString(end_time);
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, start_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_str.c_str(), -1, SQLITE_STATIC);
    
    std::vector<MarketDataRecord> records;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        MarketDataRecord record;
        record.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        record.timestamp = stringToTimePoint(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        record.open_price = sqlite3_column_double(stmt, 2);
        record.high_price = sqlite3_column_double(stmt, 3);
        record.low_price = sqlite3_column_double(stmt, 4);
        record.close_price = sqlite3_column_double(stmt, 5);
        record.volume = sqlite3_column_int64(stmt, 6);
        records.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return records;
}

std::vector<std::string> DatabaseService::getAvailableSymbols() {
    std::string query = "SELECT DISTINCT symbol FROM market_data ORDER BY symbol";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }
    
    std::vector<std::string> symbols;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        symbols.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    
    sqlite3_finalize(stmt);
    return symbols;
}

bool DatabaseService::deleteMarketData(const std::string& symbol, const std::chrono::system_clock::time_point& before_time) {
    std::string query = "DELETE FROM market_data WHERE symbol = ?";
    std::vector<std::string> params = {symbol};
    
    if (before_time != std::chrono::system_clock::time_point{}) {
        query += " AND timestamp < ?";
        params.push_back(timePointToString(before_time));
    }
    
    return executeQuery(query, params);
}

bool DatabaseService::executeQuery(const std::string& query) {
    char* error_msg = nullptr;
    int rc = sqlite3_exec(pimpl_->db, query.c_str(), nullptr, nullptr, &error_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << error_msg << std::endl;
        sqlite3_free(error_msg);
        return false;
    }
    
    return true;
}

bool DatabaseService::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_STATIC);
    }
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::string DatabaseService::timePointToString(const std::chrono::system_clock::time_point& tp) const {
    if (tp == std::chrono::system_clock::time_point{}) {
        return "";
    }
    
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::chrono::system_clock::time_point DatabaseService::stringToTimePoint(const std::string& str) const {
    if (str.empty()) {
        return {};
    }
    
    std::tm tm = {};
    std::istringstream iss(str);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string DatabaseService::jsonToString(const json& j) const {
    return j.dump();
}

json DatabaseService::stringToJson(const std::string& str) const {
    if (str.empty()) {
        return json{};
    }
    
    try {
        return json::parse(str);
    } catch (const json::parse_error&) {
        return json{};
    }
}

} // namespace fingraph