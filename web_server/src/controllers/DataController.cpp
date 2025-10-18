#include "controllers/DataController.h"
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>
#include <json/json.h>

using json = nlohmann::json;


namespace fingraph {
using namespace drogon;

DataController::DataController() : dataService_{"./uploads"} {}

void DataController::uploadData(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback) {
    MultiPartParser parser;
    if (parser.parse(req) != 0 || parser.getFiles().empty()) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        resp->setBody("No file uploaded or invalid request format");
        callback(resp);
        return;
    }

    const auto& file = parser.getFiles()[0];
    try {
        std::string dataId = dataService_.saveUploadedFile(file.getFileName(), file.fileData());
        
        Json::Value responseJson;
        responseJson["message"] = "File uploaded successfully";
        responseJson["dataId"] = dataId;
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "File upload failed: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to save file";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void DataController::listData(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback) {
    try {
        std::vector<std::string> dataIds = dataService_.listAvailableData();
        
        Json::Value responseJson(Json::arrayValue);
        for (const auto& id : dataIds) {
            responseJson.append(id);
        }
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to list data: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to retrieve data list";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

} // namespace fingraph