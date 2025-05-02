cmake_minimum_required(VERSION 3.15)
project(qase_cpp_reporter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where to find headers
include_directories(include)

# Main library
add_library(qase_reporter
	src/qase_reporter.cpp
)

include(FetchContent)

FetchContent_Declare(
	json
	URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

# Test executable
add_executable(qase_reporter_tests
	tests/test_main.cpp
)

# Link the library to the tests
target_link_libraries(qase_reporter_tests
	PRIVATE
	qase_reporter
	nlohmann_json::nlohmann_json
)
