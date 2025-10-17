#pragma once
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace fingraph {

struct OHLCV {
    std::chrono::system_clock::time_point timestamp;
    double open;
    double high;
    double low;
    double close;
    uint64_t volume;
};

class MarketData {
public:
    MarketData() = default;
    ~MarketData() = default;
    
    bool loadFromCSV(const std::string& filePath);
    const std::vector<OHLCV>& getData() const;
    std::vector<OHLCV> getDataInRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const;
    
private:
    std::vector<OHLCV> data_;
    std::map<std::chrono::system_clock::time_point, size_t> timestampIndex_;
};

} 