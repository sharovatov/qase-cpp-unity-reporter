cmake_minimum_required(VERSION 3.25)
project(qase_cpp_reporter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(QASE_SCHEMA_VALIDATION "Enable JSON Schema validation support" OFF)

include_directories(include)
cmake_policy(SET CMP0135 NEW)

include(FetchContent)

# --- Always fetch nlohmann/json ---
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install ON CACHE INTERNAL "")  # KEY: allow downstream export
FetchContent_MakeAvailable(json)

# --- Conditionally fetch and configure json-schema-validator ---
if(QASE_SCHEMA_VALIDATION)
    message(STATUS "Schema validation is enabled")

    set(JSON_SCHEMA_VALIDATOR_BUILD_TESTS OFF CACHE INTERNAL "")
    set(JSON_SCHEMA_VALIDATOR_INSTALL ON CACHE INTERNAL "")  # required to avoid export conflict

    FetchContent_Declare(
        json_schema_validator
        GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git
    )
    FetchContent_MakeAvailable(json_schema_validator)
endif()

# --- Reporter library ---
add_library(qase_reporter
    src/qase_reporter.cpp
)

target_link_libraries(qase_reporter
    PRIVATE
    nlohmann_json::nlohmann_json
)

if(QASE_SCHEMA_VALIDATION)
    target_sources(qase_reporter
        PRIVATE
        src/json_schema_validator.cpp
    )
    target_link_libraries(qase_reporter
        PRIVATE
        nlohmann_json_schema_validator
    )
    target_compile_definitions(qase_reporter PRIVATE QASE_SCHEMA_VALIDATION_ENABLED)
endif()

# --- Tests ---
add_executable(qase_reporter_tests
    tests/test_main.cpp
)

target_link_libraries(qase_reporter_tests
    PRIVATE
    qase_reporter
    nlohmann_json::nlohmann_json
)

if(QASE_SCHEMA_VALIDATION)
    target_compile_definitions(qase_reporter_tests PRIVATE QASE_SCHEMA_VALIDATION_ENABLED)
endif()
