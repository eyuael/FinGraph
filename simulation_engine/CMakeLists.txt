# Minimum version of CMake required
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


cmake_minimum_required(VERSION 3.16)

# Define the project name and language
project(FinGraphSimulation LANGUAGES CXX)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Define the include directories
include_directories(include)
include_directories(/opt/homebrew/include)

# Generate protobuf and gRPC files
set(PROTO_FILES
    proto/simulation_engine.proto
)

# Generate protobuf files
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})
    
    # Generate gRPC files
    set(GRPC_SRCS)
    set(GRPC_HDRS)
    grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILE})
    
    list(APPEND ALL_PROTO_SRCS ${PROTO_SRCS} ${GRPC_SRCS})
    list(APPEND ALL_PROTO_HDRS ${PROTO_HDRS} ${GRPC_HDRS})
endforeach()

# --- Build the Core Library ---
# This creates a static library (.a on Linux/macOS, .lib on Windows)
# named "fingraph_simulation" from all our source files.
add_library(fingraph_simulation
    src/MarketData.cpp
    src/Trade.cpp
    src/Portfolio.cpp
    src/Backtest.cpp
    src/PerformanceMetrics.cpp
    src/strategies/MovingAverageStrategy.cpp
    src/strategies/RSIStrategy.cpp
    src/JobManager.cpp
    src/SimulationEngineServer.cpp
    src/DatabaseService.cpp
    ${ALL_PROTO_SRCS}
)

# Include SQLite3 directories
target_include_directories(fingraph_simulation PRIVATE ${SQLITE3_INCLUDE_DIRS})
target_link_libraries(fingraph_simulation PRIVATE ${SQLITE3_LIBRARIES})

# --- Build the Command-Line Executable ---
# This creates an executable named "fingraph_cli" from main.cpp
add_executable(fingraph_cli src/main.cpp)

# --- Build the gRPC Server Executable ---
# This creates an executable named "fingraph_grpc_server" from the gRPC server main
add_executable(fingraph_grpc_server src/grpc_main.cpp)

# Link the CLI executable against our library.
target_link_libraries(fingraph_cli PRIVATE 
    fingraph_simulation
    nlohmann_json::nlohmann_json
)

# Link the gRPC server executable against our library and gRPC libraries
target_link_libraries(fingraph_grpc_server PRIVATE 
    fingraph_simulation
    gRPC::grpc++
    protobuf::libprotobuf
    nlohmann_json::nlohmann_json
)

# --- Testing ---
# Enable CTest, the CMake testing driver.
enable_testing()

# Add the tests subdirectory. This will look for tests/CMakeLists.txt
add_subdirectory(tests)