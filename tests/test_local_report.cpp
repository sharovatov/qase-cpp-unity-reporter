#include "qase_reporter.h"
#include "json_schema_validator.h"

void test_qase_save_report_writes_valid_schema_json() {
	qase_reporter_reset();
	qase_reporter_add_result("TDD saves lives", true);

	std::string path = "qase_test_report.json";

	// save the report
	qase_save_report(qase_reporter_get_results(), path);

	// parse the file
	nlohmann::json json_payload;
	std::ifstream in(path);
	assert(in && "expected report file to be created");

	try {
		in >> json_payload;
	} catch (const std::exception& e) {
		assert(false && "failed to parse saved JSON report");
	}
	in.close();

	// validate against schema
	try {
		validate_json_payload(json_payload);
	} catch (const std::exception& e) {
		assert(false && "saved JSON must pass schema validation");
	}

	// spot check the output
	assert(json_payload.contains("results"));
	assert(json_payload["results"].is_array());
	assert(json_payload["results"].size() == 1);
	assert(json_payload["results"][0]["title"] == "TDD saves lives");
	assert(json_payload["results"][0]["status"] == "passed");

	std::remove(path.c_str());
}

