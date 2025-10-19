#pragma once
#include <string>
#include <vector>
#include <pqxx/pqxx> // C++ library for PostgreSQL

namespace fingraph {

struct OHLCV; // Forward declare

class DatabaseManager {
public:
    explicit DatabaseManager(std::string connectionString);
    ~DatabaseManager() = default;

    // Connects to the database
    void connect();

    // Inserts a batch of OHLCV data points efficiently
    void insertMarketData(const std::string& symbol, const std::vector<OHLCV>& data);

    // Checks if data for a given symbol and date already exists
    bool dataExists(const std::string& symbol, const std::string& date);

private:
    std::string connectionString_;
    std::unique_ptr<pqxx::connection> connection_;
};

} // namespace fingraph