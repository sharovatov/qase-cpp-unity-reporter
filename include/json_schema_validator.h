#pragma once

#ifdef QASE_SCHEMA_VALIDATION

#include <string>
#include <nlohmann/json.hpp>

void validate_json_against_schema(const std::string& json_str, const nlohmann::json& schema);

#endif
