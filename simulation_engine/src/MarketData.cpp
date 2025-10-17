#include "fingraph/MarketData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <map>

namespace fingraph {

bool MarketData::loadFromCSV(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return false;
    }

    data_.clear();
    timestampIndex_.clear();
    std::string line;
    
    // Skip header row
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timestampStr, openStr, highStr, lowStr, closeStr, volumeStr;
        
        // Expected CSV format: timestamp,open,high,low,close,volume
        std::getline(ss, timestampStr, ',');
        std::getline(ss, openStr, ',');
        std::getline(ss, highStr, ',');
        std::getline(ss, lowStr, ',');
        std::getline(ss, closeStr, ',');
        std::getline(ss, volumeStr, ',');

        try {
            // Parse timestamp (assuming ISO 8601 format, e.g., "2023-01-01")
            std::tm tm = {};
            std::istringstream ss_timestamp(timestampStr);
            ss_timestamp >> std::get_time(&tm, "%Y-%m-%d");
            if (ss_timestamp.fail()) {
                throw std::runtime_error("Failed to parse timestamp: " + timestampStr);
            }
            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            // Parse numeric values
            OHLCV candle;
            candle.timestamp = timestamp;
            candle.open = std::stod(openStr);
            candle.high = std::stod(highStr);
            candle.low = std::stod(lowStr);
            candle.close = std::stod(closeStr);
            candle.volume = std::stoull(volumeStr);

            data_.push_back(candle);
            // Build the index for fast lookups
            timestampIndex_[candle.timestamp] = data_.size() - 1;

        } catch (const std::exception& e) {
            std::cerr << "Error parsing line: " << line << "\n" << e.what() << std::endl;
            // Continue to the next line instead of failing completely
        }
    }

    std::cout << "Successfully loaded " << data_.size() << " data points from " << filePath << std::endl;
    return true;
}

const std::vector<OHLCV>& MarketData::getData() const {
    return data_;
}

std::vector<OHLCV> MarketData::getDataInRange(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    std::vector<OHLCV> result;
    
    // Find the starting index using the map for O(log n) complexity
    auto startIt = timestampIndex_.lower_bound(start);
    if (startIt == timestampIndex_.end()) {
        return result; // Start date is after all data
    }

    size_t startIndex = startIt->second;
    
    // Iterate from the start index to the end of the data
    for (size_t i = startIndex; i < data_.size(); ++i) {
        if (data_[i].timestamp > end) {
            break; // We've gone past the end date
        }
        result.push_back(data_[i]);
    }
    
    return result;
}

} // namespace fingraph