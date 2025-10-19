#pragma once
#include <drogon/HttpController.h>
#include "../services/DataService.h"

namespace fingraph {
class DataController : public drogon::HttpController<DataController> {
public:
    DataController();
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DataController::uploadData, "/api/v1/data/upload", drogon::Post);
    ADD_METHOD_TO(DataController::listData, "/api/v1/data/list", drogon::Get);
    ADD_METHOD_TO(DataController::getDataPreview, "/api/v1/data/{id}/preview", drogon::Get);
    ADD_METHOD_TO(DataController::getDataMetadata, "/api/v1/data/{id}/metadata", drogon::Get);
    ADD_METHOD_TO(DataController::deleteData, "/api/v1/data/{id}", drogon::Delete);
    METHOD_LIST_END

    void uploadData(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void listData(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getDataPreview(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       const std::string& dataId);

    void getDataMetadata(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        const std::string& dataId);

    void deleteData(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    const std::string& dataId);

private:
    // In a real app, this would be injected or managed by a DI container.
    // For simplicity, we instantiate it here.
    DataService dataService_{"./uploads"};
    
    static int getTotalRows(const std::string& filePath);
    static Json::Value getDateRange(const std::string& filePath);
};

} // namespace fingraph