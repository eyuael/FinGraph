#include "../include/fingraph/DatabaseManager.h"
#include "../include/fingraph/APIClient.h"
#include <stdexcept>
#include <iostream>

namespace fingraph {

DatabaseManager::DatabaseManager(std::string connectionString) 
    : connectionString_(std::move(connectionString)) {}

void DatabaseManager::connect() {
    try {
        connection_ = std::make_unique<pqxx::connection>(connectionString_);
        if (!connection_->is_open()) {
            throw std::runtime_error("Failed to open database connection");
        }
        std::cout << "Connected to database successfully." << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Database connection failed: " + std::string(e.what()));
    }
}

void DatabaseManager::insertMarketData(const std::string& symbol, const std::vector<fingraph::OHLCV>& data) {
    if (!connection_ || !connection_->is_open()) {
        throw std::runtime_error("Database not connected");
    }

    try {
        pqxx::work txn(*connection_);
        
        for (const auto& ohlcv : data) {
            txn.exec_params(
                "INSERT INTO market_data (symbol, timestamp, open_price, high_price, low_price, close_price, volume) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7) "
                "ON CONFLICT (symbol, timestamp) DO NOTHING",
                symbol, ohlcv.timestamp, ohlcv.open, ohlcv.high, ohlcv.low, ohlcv.close, ohlcv.volume
            );
        }
        
        txn.commit();
        std::cout << "Inserted " << data.size() << " data points for " << symbol << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to insert market data: " + std::string(e.what()));
    }
}

bool DatabaseManager::dataExists(const std::string& symbol, const std::string& date) {
    if (!connection_ || !connection_->is_open()) {
        throw std::runtime_error("Database not connected");
    }

    try {
        pqxx::work txn(*connection_);
        pqxx::result result = txn.exec_params(
            "SELECT 1 FROM market_data WHERE symbol = $1 AND timestamp = $2 LIMIT 1",
            symbol, date
        );
        txn.commit();
        
        return result.size() > 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check data existence: " + std::string(e.what()));
    }
}

} // namespace fingraph