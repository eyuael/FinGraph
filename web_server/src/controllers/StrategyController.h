#pragma once
#include <drogon/HttpController.h>
#include "../services/BacktestService.h"

namespace fingraph {
class StrategyController : public drogon::HttpController<StrategyController> {
public:
    StrategyController() noexcept;
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(StrategyController::listStrategies, "/api/v1/strategies", drogon::Get);
    ADD_METHOD_TO(StrategyController::getStrategy, "/api/v1/strategies/{id}", drogon::Get);
    METHOD_LIST_END
    
    void listStrategies(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    void getStrategy(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    const std::string& strategyId);
    
private:
    BacktestService backtestService_;
    
    static std::string getStrategyDescription(const std::string& strategyName);
    static Json::Value getStrategyParameters(const std::string& strategyName);
};
}