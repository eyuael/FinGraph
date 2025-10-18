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
    METHOD_LIST_END

    void uploadData(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void listData(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    // In a real app, this would be injected or managed by a DI container.
    // For simplicity, we instantiate it here.
    DataService dataService_{"./uploads"}; 
};

} // namespace fingraph