#include <iostream>
#include <cassert>
#include <fstream> // maybe we need to include and use fstreams only when built to do so?
#include "qase_reporter.h"

using namespace qase;

// token, host and project code should be taken from the config file
void test_load_qase_config_parses_fields_correctly()
{
	const std::string config_path = "test_qase_config.json";

	// write the temporary file
	std::ofstream out(config_path);
	out << R"({
		"testops": {
			"api": {
				"token": "MY_TEST_TOKEN",
				"host": "api.qase.io"
			},
			"project": "ET-1"
		}
	})";
	out.close();

	QaseConfig cfg = load_qase_config(config_path);

	assert(cfg.token == "MY_TEST_TOKEN");
	assert(cfg.host == "api.qase.io");
	assert(cfg.project == "ET-1");

	// remove the temp file
	std::remove(config_path.c_str());
}

void test_load_qase_config_throws_if_file_missing() {
	const std::string config_path = "nonexistent_config.json";

	bool threw = false;
	try {
		load_qase_config(config_path);
	} catch (const std::runtime_error& e) {
		threw = std::string(e.what()).find("Could not open config file") != std::string::npos;
	}

	assert(threw && "Expected load_qase_config to throw if file does not exist");
}

void test_load_qase_config_throws_on_invalid_json() {
	const std::string config_path = "invalid_json_config.json";

	// write a deliberately malformed JSON
	std::ofstream out(config_path);
	out << "{ this is not valid json ";
	out.close();

	bool threw = false;
	try {
		load_qase_config(config_path);
	} catch (const std::runtime_error& e) {
		threw = std::string(e.what()).find("Failed to parse JSON") != std::string::npos;
	}

	assert(threw && "Expected load_qase_config to throw on invalid JSON");

	std::remove(config_path.c_str());
}

void test_load_qase_config_throws_on_missing_fields() {
	const std::string config_path = "missing_fields_config.json";

	std::ofstream out(config_path);
	// json with only a token, but no host or a project
	out << R"({
		"testops": {"api": {"token": "MY_TEST_TOKEN"}}
	})";
	out.close();

	bool threw = false;
	try {
		load_qase_config(config_path);
	} catch (const std::runtime_error& e) {
		std::string msg = e.what();
		threw = msg.find("Missing required field") != std::string::npos;
	}

	assert(threw && "Expected exception due to missing required fields");

	std::remove(config_path.c_str());
}

void test_load_qase_config_throws_on_empty_fields() {
	const std::string config_path = "empty_fields_config.json";

	std::ofstream out(config_path);
	out << R"({
		"testops": {
			"api": {
				"token": "",
				"host": ""
			},
			"project": ""
		}
	})";
	out.close();

	bool threw = false;
	try {
		load_qase_config(config_path);
	} catch (const std::runtime_error& e) {
		std::string msg = e.what();
		threw = msg.find("must not be empty") != std::string::npos;
	}

	assert(threw && "Expected exception due to empty config values");

	std::remove(config_path.c_str());
}
