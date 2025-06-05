#include "json_schema_validator.h"
#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace qase {

	void validate_json_payload(const json& payload) {
		static json schema;

		if (schema.empty()) {

			std::ifstream schema_file("schemas/root.json");
			if (!schema_file) {
				throw std::runtime_error("Could not open schema file");
			}

			try {
				schema_file >> schema;
			} catch (const std::exception& e) {
				throw;
			}
		}

		static json_validator validator;
		validator.set_root_schema(schema); // safe to call multiple times, but expensive — we cache

		try {
			validator.validate(payload);
		} catch (const std::exception& e) {
			throw;  // rethrow so test fails
		}
	}

}
