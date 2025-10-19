#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace fingraph {

struct OHLCV {
    std::string timestamp;
    double open;
    double high;
    double low;
    double close;
    unsigned long long volume;
};

class APIClient {
public:
    APIClient(std::string apiKey, std::string baseUrl);
    ~APIClient() = default;

    // Fetches daily data for a given symbol
    std::vector<OHLCV> getDailyTimeSeries(const std::string& symbol);

private:
    std::string apiKey_;
    std::string baseUrl_;

    // Helper to parse the JSON response from the API
    std::vector<OHLCV> parseDailyTimeSeries(const nlohmann::json& jsonResponse) const;
};

} // namespace fingraph