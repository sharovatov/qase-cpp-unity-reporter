cmake_minimum_required(VERSION 3.15)
project(qase_cpp_reporter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where to find your headers
include_directories(include)

cmake_policy(SET CMP0135 NEW)

# Download the json lib
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Build the reporter library
add_library(qase_reporter
    src/qase_reporter.cpp
)

# Link JSON to the reporter library
target_link_libraries(qase_reporter
    PRIVATE
    nlohmann_json::nlohmann_json
)

# Build the test executable
add_executable(qase_reporter_tests
    tests/test_main.cpp
)

# Link the reporter lib and JSON to the test executable
target_link_libraries(qase_reporter_tests
    PRIVATE
    qase_reporter
    nlohmann_json::nlohmann_json
)
