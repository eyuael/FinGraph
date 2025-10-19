#include <drogon/drogon.h>
#include "controllers/BacktestController.h"
#include "controllers/DataController.h"
#include "controllers/StrategyController.h"

int main() {
    // Set config file path
    drogon::app().loadConfigFile("./config/config.json");
    
    // Run HTTP server, the non-blocking way.
    // drogon::app().run() is a blocking call, so we don't need to do anything else here.
    drogon::app().run();
    
    return 0;
}