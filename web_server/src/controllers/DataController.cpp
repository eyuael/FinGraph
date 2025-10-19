#include "controllers/DataController.h"
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

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

void DataController::getDataPreview(const drogon::HttpRequestPtr& req,
                                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                     const std::string& dataId) {
    try {
        std::string dataPath = dataService_.getDataPath(dataId);
        std::ifstream file(dataPath);
        
        if (!file.is_open()) {
            Json::Value errorJson;
            errorJson["error"] = "Data file not found";
            auto resp = HttpResponse::newHttpJsonResponse(errorJson);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        std::string line;
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
        int rowCount = 0;
        const int maxRows = 10;
        
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, ',')) {
                headers.push_back(cell);
            }
        }
        
        while (std::getline(file, line) && rowCount < maxRows) {
            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> row;
            while (std::getline(ss, cell, ',')) {
                row.push_back(cell);
            }
            rows.push_back(row);
            rowCount++;
        }
        
        Json::Value responseJson;
        Json::Value headersJson(Json::arrayValue);
        for (const auto& header : headers) {
            headersJson.append(header);
        }
        responseJson["headers"] = headersJson;
        
        Json::Value rowsJson(Json::arrayValue);
        for (const auto& row : rows) {
            Json::Value rowJson(Json::arrayValue);
            for (const auto& cell : row) {
                rowJson.append(cell);
            }
            rowsJson.append(rowJson);
        }
        responseJson["rows"] = rowsJson;
        responseJson["totalRows"] = getTotalRows(dataPath);
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to preview data: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to preview data";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void DataController::getDataMetadata(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    const std::string& dataId) {
    try {
        std::string dataPath = dataService_.getDataPath(dataId);
        
        if (!std::filesystem::exists(dataPath)) {
            Json::Value errorJson;
            errorJson["error"] = "Data file not found";
            auto resp = HttpResponse::newHttpJsonResponse(errorJson);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        Json::Value responseJson;
        responseJson["dataId"] = dataId;
        responseJson["filename"] = dataId;
        responseJson["size"] = static_cast<int64_t>(std::filesystem::file_size(dataPath));
        responseJson["rows"] = getTotalRows(dataPath);
        
        auto ftime = std::filesystem::last_write_time(dataPath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        responseJson["lastModified"] = static_cast<int64_t>(sctp.time_since_epoch().count());
        
        responseJson["dateRange"] = getDateRange(dataPath);
        
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to get data metadata: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to retrieve data metadata";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void DataController::deleteData(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                               const std::string& dataId) {
    try {
        std::string dataPath = dataService_.getDataPath(dataId);
        
        if (!std::filesystem::exists(dataPath)) {
            Json::Value errorJson;
            errorJson["error"] = "Data file not found";
            auto resp = HttpResponse::newHttpJsonResponse(errorJson);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        if (std::filesystem::remove(dataPath)) {
            Json::Value responseJson;
            responseJson["message"] = "Data file deleted successfully";
            responseJson["dataId"] = dataId;
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            callback(resp);
        } else {
            Json::Value errorJson;
            errorJson["error"] = "Failed to delete data file";
            auto resp = HttpResponse::newHttpJsonResponse(errorJson);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to delete data: " << e.what();
        Json::Value errorJson;
        errorJson["error"] = "Failed to delete data file";
        auto resp = HttpResponse::newHttpJsonResponse(errorJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

int DataController::getTotalRows(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return 0;
    
    int rows = 0;
    std::string line;
    while (std::getline(file, line)) {
        rows++;
    }
    return rows > 0 ? rows - 1 : 0; // Subtract header row
}

Json::Value DataController::getDateRange(const std::string& filePath) {
    Json::Value dateRange;
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        dateRange["start"] = "";
        dateRange["end"] = "";
        return dateRange;
    }
    
    std::string firstDate, lastDate;
    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue; // Skip header
        }
        
        std::stringstream ss(line);
        std::string date;
        if (std::getline(ss, date, ',')) {
            if (firstDate.empty()) {
                firstDate = date;
            }
            lastDate = date;
        }
    }
    
    dateRange["start"] = firstDate;
    dateRange["end"] = lastDate;
    return dateRange;
}

} // namespace fingraph