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
    METHOD_LIST_END
    
    void runBacktest(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);
    
private:
    BacktestService backtestService_;
};//namespace fingraph
}