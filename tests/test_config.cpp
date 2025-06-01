#include <iostream>
#include <cassert>
#include <fstream>
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
			"project": "ET-1",
			"run": {
				"id": 4242
			}
		}
	})";
	out.close();

	QaseConfig cfg = load_qase_config_from_file(config_path);

	assert(cfg.token == "MY_TEST_TOKEN");
	assert(cfg.host == "api.qase.io");
	assert(cfg.project == "ET-1");
	assert(cfg.run_id == 4242);

	// remove the temp file
	std::remove(config_path.c_str());
}

void test_load_qase_config_throws_if_file_missing() {
	const std::string config_path = "nonexistent_config.json";

	bool threw = false;
	try {
		load_qase_config_from_file(config_path);
	} catch (const std::runtime_error& e) {
		threw = std::string(e.what()).find("Could not open config file") != std::string::npos;
	}

	assert(threw && "Expected load_qase_config_from_file to throw if file does not exist");
}

void test_load_qase_config_throws_on_invalid_json() {
	const std::string config_path = "invalid_json_config.json";

	// write a deliberately malformed JSON
	std::ofstream out(config_path);
	out << "{ this is not valid json ";
	out.close();

	bool threw = false;
	try {
		load_qase_config_from_file(config_path);
	} catch (const std::runtime_error& e) {
		threw = std::string(e.what()).find("Failed to parse JSON") != std::string::npos;
	}

	assert(threw && "Expected load_qase_config_from_file to throw on invalid JSON");

	std::remove(config_path.c_str());
}

void test_load_qase_config_throws_on_missing_fields() {
	const std::string config_path = "missing_fields_config.json";

	std::ofstream out(config_path);
	// neither token nor project are given!
	out << R"({
		"testops": {"api": {}}
	})";
	out.close();

	bool threw = false;
	try {
		load_qase_config_from_file(config_path);
	} catch (const std::runtime_error& e) {
		std::string msg = e.what();
		threw = msg.find("Missing required field") != std::string::npos;
	}

	assert(threw && "Expected exception due to missing required fields: token and project");

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
		load_qase_config_from_file(config_path);
	} catch (const std::runtime_error& e) {
		std::string msg = e.what();
		threw = msg.find("must not be empty") != std::string::npos;
	}

	assert(threw && "Expected exception due to empty config values");

	std::remove(config_path.c_str());
}

void test_resolve_config_uses_file_if_nothing_else() {
	const std::string config_path = "file_only_config.json";

	std::ofstream out(config_path);
	out << R"({
		"testops": {
			"api": {
				"token": "token_from_file",
				"host": "host_from_file"
			},
			"project": "project_from_file"
		}
	})";
	out.close();

	ConfigResolutionInput input;
	input.file = config_path;

	QaseConfig cfg = resolve_config(input);

	assert(cfg.token == "token_from_file");
	assert(cfg.host == "host_from_file");
	assert(cfg.project == "project_from_file");

	std::remove(config_path.c_str());
}

void test_resolve_config_env_vars_override_file() {
	const std::string config_path = "config_with_defaults.json";

	std::ofstream out(config_path);
	out << R"({
        "testops": {
            "api": {
                "token": "from_file_token",
                "host": "from_file_host"
            },
            "project": "from_file_project"
        }
    })";
	out.close();

	// envs should override the file
	setenv("QASE_TOKEN", "from_env_token", 1);
	setenv("QASE_HOST", "from_env_host", 1);
	setenv("QASE_PROJECT", "from_env_project", 1);

	ConfigResolutionInput input;
	input.env_prefix = "QASE_";
	input.file = config_path;

	QaseConfig cfg = resolve_config(input);

	assert(cfg.token == "from_env_token");
	assert(cfg.host == "from_env_host");
	assert(cfg.project == "from_env_project");

	std::remove(config_path.c_str());
	unsetenv("QASE_TOKEN");
	unsetenv("QASE_HOST");
	unsetenv("QASE_PROJECT");
}

void test_resolve_config_preset_overrides_env_and_file() {
	const std::string config_path = "test_config_priority.json";
	std::ofstream out(config_path);
	out << R"({
        "testops": {
            "api": {
                "token": "file_token",
                "host": "file_host"
            },
            "project": "file_project"
        }
    })";
	out.close();

	setenv("QASE_TOKEN", "env_token", 1);
	setenv("QASE_HOST", "env_host", 1);
	setenv("QASE_PROJECT", "env_project", 1);

	// this preconfigured QaseConfig obj should override anything else!
	QaseConfig preset;
	preset.token = "preset_token";
	preset.host = "preset_host";
	preset.project = "preset_project";

	ConfigResolutionInput input;
	input.file = config_path;
	input.env_prefix = "QASE_";
	input.preset = preset;

	QaseConfig cfg = resolve_config(input);

	assert(cfg.token == "preset_token");
	assert(cfg.host == "preset_host");
	assert(cfg.project == "preset_project");

	std::remove(config_path.c_str());
}

void test_merge_config_overrides_strings() {
	QaseConfig base;
	base.token = "base_token";
	base.project = "base_project";
	base.host = "base_host";

	QaseConfig incoming;
	incoming.token = "override_token";
	incoming.host = "override_host";
	// project left empty on purpose

	QaseConfig merged = merge_config(base, incoming);

	assert(merged.token == "override_token");   // overridden
	assert(merged.host == "override_host");     // overridden
	assert(merged.project == "base_project");   // unchanged

	// base must remain untouched
	assert(base.token == "base_token");
	assert(base.host == "base_host");
	assert(base.project == "base_project");
}

void test_load_qase_config_from_env_reads_expected_fields() {
	setenv("QASE_TOKEN", "token_from_env", 1);
	setenv("QASE_HOST", "host_from_env", 1);
	setenv("QASE_PROJECT", "project_from_env", 1);

	QaseConfig cfg = load_qase_config_from_env("QASE_");

	assert(cfg.token == "token_from_env");
	assert(cfg.host == "host_from_env");
	assert(cfg.project == "project_from_env");

	unsetenv("QASE_TOKEN");
	unsetenv("QASE_HOST");
	unsetenv("QASE_PROJECT");
}
