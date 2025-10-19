#pragma once
#include <drogon/HttpController.h>
#include "services/BacktestService.h"

using namespace drogon;
namespace fingraph {
class BacktestController : public drogon::HttpController<BacktestController> {
public:
    BacktestController() noexcept;
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(BacktestController::runBacktest, "/api/v1/backtest", Post);
    ADD_METHOD_TO(BacktestController::getBacktest, "/api/v1/backtest/{id}", drogon::Get);
    ADD_METHOD_TO(BacktestController::listBacktests, "/api/v1/backtest", drogon::Get);
    METHOD_LIST_END
    
    void runBacktest(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);
    
    void getBacktest(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    const std::string& backtestId);
    
    void listBacktests(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback);
    
private:
    BacktestService backtestService_;
};//namespace fingraph
}