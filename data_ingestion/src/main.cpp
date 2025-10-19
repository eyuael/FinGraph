#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../include/fingraph/APIClient.h"
#include <filesystem>

using json = nlohmann::json;

void writeDataToCSV(const std::string& filePath, const std::vector<fingraph::OHLCV>& data) {
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    // Write CSV header
    outFile << "timestamp,open,high,low,close,volume\n";

    // Write data rows
    for (const auto& candle : data) {
        outFile << candle.timestamp << ","
                << candle.open << ","
                << candle.high << ","
                << candle.low << ","
                << candle.close << ","
                << candle.volume << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <SYMBOL>" << std::endl;
        return 1;
    }

    std::string symbol = argv[1];

    // 1. Read configuration
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config.json" << std::endl;
        return 1;
    }
    json config;
    configFile >> config;

    std::string apiKey = config["alpha_vantage"]["api_key"];
    std::string baseUrl = config["alpha_vantage"]["base_url"];
    std::string outputDir = config["output"]["directory"];

    if (apiKey == "YOUR_API_KEY_HERE") {
        std::cerr << "Error: Please replace 'YOUR_API_KEY_HERE' in config.json with your actual Alpha Vantage API key." << std::endl;
        return 1;
    }

    // 2. Fetch data from API
    fingraph::APIClient client(apiKey, baseUrl);
    std::vector<fingraph::OHLCV> marketData = client.getDailyTimeSeries(symbol);

    if (marketData.empty()) {
        std::cerr << "Failed to retrieve data for symbol: " << symbol << std::endl;
        return 1;
    }

    // 3. Save data to CSV file
    try {
        // Ensure the output directory exists
        std::filesystem::create_directories(outputDir);
        
        std::string outputPath = outputDir + "/" + symbol + ".csv";
        writeDataToCSV(outputPath, marketData);
        
        std::cout << "Successfully saved data for " << symbol << " to " << outputPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}