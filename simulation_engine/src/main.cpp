#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "fingraph/Backtest.h"

// For convenience
using json = nlohmann::json;
using namespace fingraph;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file.json>" << std::endl;
        return 1;
    }
    
    try {
        // 1. Read and Parse Configuration
        std::ifstream configFile(argv[1]);
        if (!configFile.is_open()) {
            throw std::runtime_error("Could not open config file: " + std::string(argv[1]));
        }
        
        json config;
        configFile >> config;

        // 2. Initialize and Run Backtest Engine
        BacktestEngine engine;
        
        BacktestResult result = engine.runBacktest(
            config["dataPath"],
            config["strategy"],
            config["parameters"],
            config["initialCash"]
        );
        
        // 3. Serialize Results to JSON
        json output;
        output["totalReturn"] = result.totalReturn;
        output["sharpeRatio"] = result.sharpeRatio;
        output["maxDrawdown"] = result.maxDrawdown;
        output["winRate"] = result.winRate;
        
        json trades = json::array();
        for (const auto& trade : result.trades) {
            json t;
            t["symbol"] = trade.getSymbol();
            t["type"] = (trade.getType() == TradeType::BUY) ? "BUY" : "SELL";
            t["quantity"] = trade.getQuantity();
            t["price"] = trade.getPrice();
            t["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                trade.getTimestamp().time_since_epoch()).count();
            trades.push_back(t);
        }
        output["trades"] = trades;
        
        json equityCurve = json::array();
        for (const auto& point : result.equityCurve) {
            json p;
            p["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                point.first.time_since_epoch()).count();
            p["value"] = point.second;
            equityCurve.push_back(p);
        }
        output["equityCurve"] = equityCurve;
        
        // 4. Print JSON to Standard Output
        std::cout << output.dump(4) << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        // Print errors to standard error so they don't get mixed with JSON output
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}