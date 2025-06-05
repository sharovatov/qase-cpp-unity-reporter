#include <cassert>
#include <nlohmann/json.hpp>
#include "json_schema_validator.h"

using nlohmann::json;

void test_valid_json_passes_schema() {
	json valid = {
		{"title", "Sample Report"},
		{"execution", {
			{"start_time", 1000},
			{"end_time", 2000},
			{"duration", 1000},
			{"cumulative_duration", 1000}
		}},
		{"stats", {
			{"total", 1},
			{"passed", 1},
			{"failed", 0},
			{"skipped", 0},
			{"broken", 0},
			{"muted", 0}
		}},
		{"results", {{
			{"duration", 1000},
			{"id", "TC-1"},
			{"status", "passed"},
			{"thread", nullptr},
			{"title", "Test case 1"}
		}}},
		{"threads", json::array()},
		{"suites", json::array()},
		{"environment", nullptr},
		{"host_data", {{
			{"name", "os"},
			{"value", "macOS"}
		}}}
	};

	try {
		validate_json_payload(valid);
	} catch (const std::exception& e) {
		assert(false && "Expected valid JSON to pass validation");
	}
}

void test_invalid_json_fails_schema() {

	json invalid = {
		{"results", "this should be an array"}
	};

	bool threw = false;
	try {
		validate_json_payload(invalid);
	} catch (const std::exception& e) {
		threw = true;
	}

	assert(threw && "Expected invalid JSON to fail schema validation");
}

