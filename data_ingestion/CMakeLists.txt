set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.16)
project(FinGraphDataIngestion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include simulation engine headers
include_directories(../simulation_engine/include)
include_directories(include)

# Add Homebrew include paths for clangd
include_directories(/opt/homebrew/include)
include_directories(/opt/homebrew/Cellar/nlohmann-json/3.12.0/include)
include_directories(/opt/homebrew/Cellar/libpqxx/7.10.3/include)
include_directories(/opt/homebrew/Cellar/cpr/1.10.5/include)

# Find required packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)

# Add the executable
add_executable(data_ingestion_service
    src/main.cpp
    src/APIClient.cpp
    src/DatabaseManager.cpp
)

# Link the libraries
target_link_libraries(data_ingestion_service PRIVATE 
    nlohmann_json::nlohmann_json
    cpr::cpr
)