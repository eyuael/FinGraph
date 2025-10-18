# Minimum version of CMake required
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


cmake_minimum_required(VERSION 3.16)

# Define the project name and language
project(FinGraphSimulation LANGUAGES CXX)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the include directories
include_directories(include)
include_directories(/opt/homebrew/include)

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
)

# --- Build the Command-Line Executable ---
# This creates an executable named "fingraph_cli" from main.cpp
add_executable(fingraph_cli src/main.cpp)

# Link the executable against our library.
# The CLI tool now has access to all the code in the library.
target_link_libraries(fingraph_cli PRIVATE fingraph_simulation)

# --- Testing ---
# Enable CTest, the CMake testing driver.
enable_testing()

# Add the tests subdirectory. This will look for tests/CMakeLists.txt
add_subdirectory(tests)