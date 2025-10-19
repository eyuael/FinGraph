#include "../include/fingraph/APIClient.h"
#include <cpr/cpr.h>
#include <iostream>


namespace fingraph {

APIClient::APIClient(std::string apiKey, std::string baseUrl)
    : apiKey_(std::move(apiKey)), baseUrl_(std::move(baseUrl)) {}

std::vector<OHLCV> APIClient::getDailyTimeSeries(const std::string& symbol) {
    // Construct the API request URL
    std::string url = baseUrl_ + "?function=TIME_SERIES_DAILY&symbol=" + symbol + "&apikey=" + apiKey_ + "&outputsize=full";
    
    std::cout << "Fetching data for " << symbol << "..." << std::endl;
    
    // Make the GET request using cpr
    cpr::Response r = cpr::Get(cpr::Url{url});

    // Check for a successful response
    if (r.status_code != 200) {
        std::cerr << "Error: API request failed with status code " << r.status_code << std::endl;
        std::cerr << "Response: " << r.text << std::endl;
        return {};
    }

    // Parse the JSON response
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(r.text);
        
        // Check for API error messages
        if (jsonResponse.contains("Error Message")) {
            std::cerr << "API Error: " << jsonResponse["Error Message"].get<std::string>() << std::endl;
            return {};
        }
        
        return parseDailyTimeSeries(jsonResponse);

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
        return {};
    }
}

std::vector<OHLCV> APIClient::parseDailyTimeSeries(const nlohmann::json& jsonResponse) const {
    std::vector<OHLCV> data;
    
    // The actual time series data is nested under the "Time Series (Daily)" key
    if (!jsonResponse.contains("Time Series (Daily)")) {
        std::cerr << "Error: JSON response does not contain 'Time Series (Daily)' key." << std::endl;
        return {};
    }
    
    const auto& timeSeries = jsonResponse["Time Series (Daily)"];
    
    // Iterate through each date in the time series
    for (auto it = timeSeries.rbegin(); it != timeSeries.rend(); ++it) { // Reverse iterator for chronological order
        const std::string& date = it.key();
        const auto& values = it.value();
        
        try {
            OHLCV candle;
            candle.timestamp = date;
            candle.open = std::stod(values["1. open"].get<std::string>());
            candle.high = std::stod(values["2. high"].get<std::string>());
            candle.low = std::stod(values["3. low"].get<std::string>());
            candle.close = std::stod(values["4. close"].get<std::string>());
            candle.volume = std::stoull(values["5. volume"].get<std::string>());
            
            data.push_back(candle);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing data for date " << date << ": " << e.what() << std::endl;
            // Skip this entry and continue
        }
    }
    
    std::cout << "Successfully parsed " << data.size() << " data points." << std::endl;
    return data;
}

} // namespace fingraph