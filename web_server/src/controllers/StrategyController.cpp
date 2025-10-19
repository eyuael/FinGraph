#include "controllers/StrategyController.h"
#include <drogon/HttpTypes.h>
#include <json/json.h>

namespace fingraph {
using namespace drogon;

StrategyController::StrategyController() noexcept : backtestService_("../simulation_engine/build/fingraph_cli", "./uploads") {}

void StrategyController::listStrategies(const HttpRequestPtr& req,
                                       std::function<void(const HttpResponsePtr&)>&& callback) {
    try {
        auto strategies = BacktestService::getAvailableStrategies();
        
        Json::Value responseJson(Json::arrayValue);
        for (const auto& strategy : strategies) {
            Json::Value strategyJson;
            strategyJson["id"] = strategy;
            strategyJson["name"] = strategy;
            strategyJson["description"] = getStrategyDescription(strategy);
            responseJson.append(strategyJson);
        }
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to list strategies: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to retrieve strategies";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void StrategyController::getStrategy(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    const std::string& strategyId) {
    try {
        auto strategies = BacktestService::getAvailableStrategies();
        bool found = std::find(strategies.begin(), strategies.end(), strategyId) != strategies.end();
        
        if (!found) {
            Json::Value errorJson;
            errorJson["error"] = "Strategy not found";
            auto resp = HttpResponse::newHttpJsonResponse(errorJson);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        Json::Value responseJson;
        responseJson["id"] = strategyId;
        responseJson["name"] = strategyId;
        responseJson["description"] = getStrategyDescription(strategyId);
        responseJson["parameters"] = getStrategyParameters(strategyId);
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to get strategy: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to retrieve strategy";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

std::string StrategyController::getStrategyDescription(const std::string& strategyName) {
    if (strategyName == "MovingAverageStrategy") {
        return "Moving Average Crossover strategy that generates buy signals when short-term MA crosses above long-term MA and sell signals for the opposite.";
    } else if (strategyName == "RSIStrategy") {
        return "RSI Mean Reversion strategy that buys when RSI is oversold and sells when RSI is overbought.";
    }
    return "Trading strategy implementation";
}

Json::Value StrategyController::getStrategyParameters(const std::string& strategyName) {
    Json::Value parameters(Json::arrayValue);
    
    if (strategyName == "MovingAverageStrategy") {
        Json::Value param1;
        param1["name"] = "shortWindow";
        param1["type"] = "integer";
        param1["defaultValue"] = 10;
        param1["min"] = 1;
        param1["max"] = 50;
        param1["description"] = "Short-term moving average window";
        parameters.append(param1);
        
        Json::Value param2;
        param2["name"] = "longWindow";
        param2["type"] = "integer";
        param2["defaultValue"] = 30;
        param2["min"] = 10;
        param2["max"] = 200;
        param2["description"] = "Long-term moving average window";
        parameters.append(param2);
    } else if (strategyName == "RSIStrategy") {
        Json::Value param1;
        param1["name"] = "period";
        param1["type"] = "integer";
        param1["defaultValue"] = 14;
        param1["min"] = 2;
        param1["max"] = 50;
        param1["description"] = "RSI calculation period";
        parameters.append(param1);
        
        Json::Value param2;
        param2["name"] = "oversoldThreshold";
        param2["type"] = "number";
        param2["defaultValue"] = 30.0;
        param2["min"] = 10.0;
        param2["max"] = 40.0;
        param2["description"] = "RSI oversold threshold for buy signals";
        parameters.append(param2);
        
        Json::Value param3;
        param3["name"] = "overboughtThreshold";
        param3["type"] = "number";
        param3["defaultValue"] = 70.0;
        param3["min"] = 60.0;
        param3["max"] = 90.0;
        param3["description"] = "RSI overbought threshold for sell signals";
        parameters.append(param3);
    }
    
    return parameters;
}

} // namespace fingraph