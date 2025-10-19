set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.16)
project(FinGraphWebServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Include simulation engine headers
include_directories(../simulation_engine/include)
include_directories(src)
# Link against the simulation engine library
link_directories(../simulation_engine/build)

add_executable(FinGraphWebServer
    src/main.cpp
    src/controllers/BacktestController.cpp
    src/controllers/DataController.cpp
    src/controllers/StrategyController.cpp
    src/services/BacktestService.cpp
    src/services/DataService.cpp
    src/utils/SimulationEngineClient.cpp
)

target_link_libraries(FinGraphWebServer PRIVATE 
    Drogon::Drogon 
    nlohmann_json::nlohmann_json
    fingraph_simulation # Link our static library
)