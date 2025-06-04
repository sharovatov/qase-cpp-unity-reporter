#ifdef QASE_SCHEMA_VALIDATION

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include "json_schema_validator.h"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

void validate_json_against_schema(const std::string& json_str, const json& schema) {
	json instance = json::parse(json_str);
	json_validator validator;
	validator.set_root_schema(schema);  // throws if invalid schema
	validator.validate(instance);       // throws if not conforming
}

#endif
