#include "controllers/BacktestController.h"
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>
#include <json/json.h>

using json = nlohmann::json;

namespace fingraph {
using namespace drogon;

// The path to the simulation engine executable.
// In a real deployment, this should be configurable.
const std::string SIMULATION_ENGINE_PATH = "../simulation_engine/build/fingraph_cli";
const std::string DATA_DIRECTORY = "./uploads";

BacktestController::BacktestController() noexcept : backtestService_(SIMULATION_ENGINE_PATH, DATA_DIRECTORY) {}

void BacktestController::runBacktest(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback) {
    auto jsonBody = req->getJsonObject();
    
    if (!jsonBody) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        resp->setBody("Invalid JSON request");
        callback(resp);
        return;
    }
    
    try {
        // Extract parameters from the request
        std::string dataId = (*jsonBody)["dataId"].asString();
        std::string strategyName = (*jsonBody)["strategy"].asString();
        double initialCash = (*jsonBody)["initialCash"].asDouble();
        
        std::map<std::string, double> strategyParams;
        if (jsonBody->isMember("parameters")) {
            for (const auto& key : (*jsonBody)["parameters"].getMemberNames()) {
                strategyParams[key] = (*jsonBody)["parameters"][key].asDouble();
            }
        }
        
        // Run the backtest via the service
        auto result = backtestService_.runBacktest(dataId, strategyName, strategyParams, initialCash);
        
        // Convert nlohmann::json to Json::Value for Drogon
        Json::Value drogonResult;
        Json::Reader reader;
        reader.parse(result.dump(), drogonResult);
        
        // Return the successful result
        auto resp = HttpResponse::newHttpJsonResponse(drogonResult);
        callback(resp);

    } catch (const std::exception& e) {
        LOG_ERROR << "Backtest failed: " << e.what();
        Json::Value errorResponse;
        errorResponse["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(errorResponse);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

} // namespace fingraph