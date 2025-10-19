#include "controllers/BacktestController.h"
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>
#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <chrono>

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

void BacktestController::getBacktest(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     const std::string& backtestId) {
    try {
        std::string resultsPath = "./results/" + backtestId + ".json";
        std::ifstream file(resultsPath);
        
        if (!file.is_open()) {
            Json::Value errorResponse;
            errorResponse["error"] = "Backtest not found";
            auto resp = HttpResponse::newHttpJsonResponse(errorResponse);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        Json::Value resultJson;
        Json::Reader reader;
        if (!reader.parse(file, resultJson)) {
            Json::Value errorResponse;
            errorResponse["error"] = "Failed to parse backtest results";
            auto resp = HttpResponse::newHttpJsonResponse(errorResponse);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
            return;
        }
        
        auto resp = HttpResponse::newHttpJsonResponse(resultJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to get backtest: " << e.what();
        Json::Value errorResponse;
        errorResponse["error"] = "Failed to retrieve backtest results";
        auto resp = HttpResponse::newHttpJsonResponse(errorResponse);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void BacktestController::listBacktests(const HttpRequestPtr& req,
                                       std::function<void(const HttpResponsePtr&)>&& callback) {
    try {
        std::string resultsDir = "./results";
        std::vector<std::string> backtestIds;
        
        if (std::filesystem::exists(resultsDir) && std::filesystem::is_directory(resultsDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(resultsDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::string filename = entry.path().filename().stem().string();
                    backtestIds.push_back(filename);
                }
            }
        }
        
        Json::Value responseJson(Json::arrayValue);
        for (const auto& id : backtestIds) {
            Json::Value backtestInfo;
            backtestInfo["id"] = id;
            auto ftime = std::filesystem::last_write_time("./results/" + id + ".json");
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            backtestInfo["timestamp"] = static_cast<int64_t>(sctp.time_since_epoch().count());
            responseJson.append(backtestInfo);
        }
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to list backtests: " << e.what();
        Json::Value errorResponse;
        errorResponse["error"] = "Failed to retrieve backtest list";
        auto resp = HttpResponse::newHttpJsonResponse(errorResponse);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

} // namespace fingraph