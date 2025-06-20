#include <nlohmann/json.hpp>
#include "qase_reporter.h"
#ifndef ESP_PLATFORM
// fstream is used only in config file reader
// and config file reader is not supported on ESP32
#include <fstream>
#include <iostream>
#include <filesystem>
#endif

using json = nlohmann::json;

namespace qase {

	static std::vector<TestResult> collected;

	void check_qase_api_error(const nlohmann::json& json)
	{
		if (json.contains("status") && json["status"].is_boolean() && json["status"] == false) {
			std::string error_message = "Unknown API error";

			if (json.contains("errorMessage") && json["errorMessage"].is_string()) {
				error_message = json["errorMessage"].get<std::string>();
			}

			throw std::runtime_error("Qase API error: " + error_message);
		}
	}

	void qase_reporter_add_result(const std::string& name, bool passed) {
		QaseResultMeta meta;
		qase_reporter_add_result(name, passed, meta);
	}

	void qase_reporter_add_result(const std::string& name, bool passed, const QaseResultMeta& meta) {
		if (name.empty()) {
			throw std::invalid_argument("Test name must not be empty");
		}
		collected.push_back(TestResult{name, passed, meta});
	}

	const std::vector<TestResult>& qase_reporter_get_results() {
		return collected;
	}

	void qase_reporter_reset() {
		collected.clear();
	}

	std::string qase_serialize_results(const std::vector<TestResult>& collected) {
		json root;
		root["results"] = json::array();

		for (const auto& result : collected) {
			json entry;
			json case_json;

			// use title from meta if provided, else - result.name
			if (!result.meta.title.empty()) {
				case_json["title"] = result.meta.title;
			} else {
				case_json["title"] = result.name;
			}

			// add case_id if set
			if (result.meta.case_id > 0) {
				case_json["case_id"] = result.meta.case_id;
			}

			// add custom fields if present
			for (const auto& kv : result.meta.fields) {
				case_json[kv.first] = kv.second;
			}

			entry["case"] = case_json;
			entry["status"] = result.passed ? "passed" : "failed";

			root["results"].push_back(entry);
		}

		return root.dump();
	}

	// helper: builds vector of headers for the specified token
	std::vector<std::string> make_headers(const std::string& token) {
		return {
			"accept: application/json",
			"content-type: application/json",
			"Token: " + token
		};
	}

	inline std::string qase_api_base(const QaseConfig& cfg) {
		return "https://" + cfg.host + "/v1/";
	}

	// qase_start_run should call Qase API and return new test run
	uint64_t QaseApi::qase_start_run(HttpClient& http, const QaseConfig& cfg) {
		const std::string url = qase_api_base(cfg) + "run/" + cfg.project;
		nlohmann::json payload;
		payload["title"] = cfg.run_title;

		// todo: support passing from the config
		payload["include_all_cases"] = true;

		if (!cfg.run_description.empty()) {
			payload["description"] = cfg.run_description;
		}

		const auto headers = make_headers(cfg.token);

		std::string response = http.post(url, payload.dump(), headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		// extract result.id if present
		if (json.contains("result") && json["result"].contains("id")) {
			return json["result"]["id"].get<uint64_t>();
		}

		throw std::runtime_error("Qase API response missing result.id field");
	}

	bool QaseApi::qase_submit_results(HttpClient& http, const QaseConfig& cfg, uint64_t run_id, const std::string& payload) {

		const std::string url = qase_api_base(cfg) + "result/" + cfg.project + "/" + std::to_string(run_id) + "/bulk";
		const auto headers = make_headers(cfg.token);

		std::string response = http.post(url, payload, headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		return json.contains("status") && json["status"] == true;

	}

	bool QaseApi::qase_complete_run(HttpClient& http, const QaseConfig& cfg, uint64_t run_id) {

		const std::string url = qase_api_base(cfg) + "run/" + cfg.project + "/" + std::to_string(run_id) + "/complete";

		const auto headers = make_headers(cfg.token);

		std::string response = http.post(url, "", headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		return json.contains("status") && json["status"] == true;

	}

	// qase_submit_report must follow this flow:
	// 1. take all the results accumulated from qase_reporter_add_result calls
	// 2. start test run in Qase API with qase_start_run
	// 3. bulk submit all serialized results to Qase API with qase_submit_results
	// 4. complete test run in Qase API with qase_complete_run
	void qase_submit_report(
			IQaseApi& api,
			HttpClient& http,
			const QaseConfig& cfg
		) {

		// step 0: take all the results accumulated from qase_reporter_add_result calls
		const auto& results = qase_reporter_get_results();
		if (results.empty()) {
			return; // nothing to submit, skip orchestration
		}

		// step 1: take all serialised results accumulated from qase_reporter_add_result calls
		const std::string payload = qase_serialize_results(results);

		// step 2: if run_id is sent from the config, use it
		// if no run_id is sent, start new test run in Qase API with qase_start_run 
		// and get the run_id of this new run
		uint64_t run_id = cfg.run_id;
		if (run_id == 0) {
			run_id = api.qase_start_run(http, cfg);
		}

		// step 3: bulk submit all serialized results to Qase API with qase_submit_results
		bool bulk_result = api.qase_submit_results(http, cfg, run_id, payload);

		// step 4: complete test run in Qase API with qase_complete_run
		// but do it only if the config doesn't prohibit this
		if (cfg.run_complete) {
			api.qase_complete_run(http, cfg, run_id);
		}

	}

	

	// ========= READING CONFIG FROM A FILE IS NOT AVAILABLE ON ESP32 =======
	#ifndef ESP_PLATFORM
	QaseConfig load_qase_config_from_file(const std::string& path) {
		std::ifstream f(path);
		if (!f.is_open()) {
			throw std::runtime_error("Could not open config file: " + path);
		}

		nlohmann::json j;
		try {
			f >> j;
		} catch (const nlohmann::json::parse_error& e) {
			throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
		}

		if (!j.contains("testops")) {
			throw std::runtime_error("Missing required field: testops");
		}
		auto& testops = j["testops"];

		if (!testops.contains("api") || !testops["api"].contains("token")) {
			throw std::runtime_error("Missing required field: testops.api.token");
		}

		const auto& tapi = testops["api"];
		std::string token = tapi["token"].get<std::string>();
		if (token.empty()) {
			throw std::runtime_error("Config field 'token' must not be empty");
		}

		if (!testops.contains("project")) {
			throw std::runtime_error("Missing required field: testops.project");
		}
		std::string project = testops["project"].get<std::string>();
		if (project.empty()) {
			throw std::runtime_error("Config field 'project' must not be empty");
		}

		// config with defaults but overriden where applicable
		QaseConfig cfg;
		cfg.token = token;
		cfg.project = project;

		if (tapi.contains("host")) {
			cfg.host = tapi["host"].get<std::string>();
		}
		if (testops.contains("run") && testops["run"].contains("complete") &&
				testops["run"]["complete"].is_boolean()) {
			cfg.run_complete = testops["run"]["complete"].get<bool>();
		}

		if (j.contains("mode")) cfg.mode = j["mode"].get<std::string>();
		if (j.contains("fallback")) cfg.fallback = j["fallback"].get<std::string>();
		if (j.contains("environment")) cfg.environment = j["environment"].get<std::string>();
		if (j.contains("rootSuite")) cfg.root_suite = j["rootSuite"].get<std::string>();

		if (j.contains("debug") && j["debug"].is_boolean()) {
			cfg.debug = j["debug"].get<bool>();
		}

		if (j.contains("captureLogs") && j["captureLogs"].is_boolean()) {
			cfg.capture_logs = j["captureLogs"].get<bool>();
		}

		if (testops.contains("report") && testops["report"].contains("driver")) {
			cfg.report_driver = testops["report"]["driver"].get<std::string>();
		}

		if (testops.contains("report") && testops["report"].contains("connection") &&
				testops["report"]["connection"].contains("path")) {
			cfg.report_connection_path = testops["report"]["connection"]["path"].get<std::string>();
		}

		if (testops.contains("report") && testops["report"].contains("connection") &&
				testops["report"]["connection"].contains("format")) {
			cfg.connection_format = testops["report"]["connection"]["format"].get<std::string>();
		}

		if (tapi.contains("enterprise") && tapi["enterprise"].is_boolean()) {
			cfg.enterprise = tapi["enterprise"].get<bool>();
		}

		// todo: support this further down the logics
		if (testops.contains("defect") && testops["defect"].is_boolean()) {
			cfg.defect = testops["defect"].get<bool>();
		}

		// todo: support passing run_id further down in the logics
		if (testops.contains("run") && testops["run"].contains("id") && testops["run"]["id"].is_number_integer()) {
			cfg.run_id = testops["run"]["id"].get<int>();
		}

		if (testops.contains("run") && testops["run"].contains("title")) {
			cfg.run_title = testops["run"]["title"].get<std::string>();
		}

		// todo: support passing run_description further down in the logics
		if (testops.contains("run") && testops["run"].contains("description")) {
			cfg.run_description = testops["run"]["description"].get<std::string>();
		}

		// todo: support passing plan_id further down in the logics
		if (testops.contains("plan") && testops["plan"].contains("id")) {
			cfg.plan_id = testops["plan"]["id"].get<int>();
		}

		// todo: support passing batch_size further down in the logics
		if (testops.contains("batch") && testops["batch"].contains("size")) {
			cfg.batch_size = testops["batch"]["size"].get<int>();
		}

		return cfg;
	}
	#endif

	// todo: implement passing more values from env?
	QaseConfig load_qase_config_from_env(const std::string& prefix) {
		QaseConfig cfg;

		const char* token = std::getenv((prefix + "TOKEN").c_str());
		if (token) cfg.token = token;

		const char* host = std::getenv((prefix + "HOST").c_str());
		if (host) cfg.host = host;

		const char* project = std::getenv((prefix + "PROJECT").c_str());
		if (project) cfg.project = project;

		const char* complete = std::getenv((prefix + "RUN_COMPLETE").c_str());
		if (complete) cfg.run_complete = std::string(complete) == "true";

		return cfg;
	}

	QaseConfig resolve_config(const ConfigResolutionInput& input) {

		// 1. start with defaults (already set in QaseConfig struct in .h file)
		QaseConfig cfg;

		#ifndef ESP_PLATFORM
		if (input.file.has_value()) {
			// 2. if there is a file, use its values to override
			// NOTE file loading support doesn't work on ESP32
			try {
				QaseConfig file_cfg = load_qase_config_from_file(input.file.value());
				cfg = merge_config(cfg, file_cfg);
			} catch (const std::exception& e) {
				throw std::runtime_error("Failed to load config from file: " + std::string(e.what()));
			}
		}
		#endif

		// 3. if there are envs, use the values to override
		if (input.env_prefix.has_value()) {
			QaseConfig env_cfg = load_qase_config_from_env(input.env_prefix.value());
			cfg = merge_config(cfg, env_cfg);
		}

		// 4. if QaseConfig instance was passed, use it to override
		if (input.preset.has_value()) {
			cfg = merge_config(cfg, input.preset.value());
		}

		return cfg;
	}

	QaseConfig merge_config(const QaseConfig& base, const QaseConfig& incoming) {
		QaseConfig result = base;

		if (!incoming.token.empty()) result.token = incoming.token;
		if (!incoming.project.empty()) result.project = incoming.project;
		if (!incoming.host.empty()) result.host = incoming.host;

		if (!incoming.mode.empty()) result.mode = incoming.mode;
		if (!incoming.fallback.empty()) result.fallback = incoming.fallback;
		if (!incoming.environment.empty()) result.environment = incoming.environment;
		if (!incoming.root_suite.empty()) result.root_suite = incoming.root_suite;

		if (incoming.debug) result.debug = true;
		if (incoming.capture_logs) result.capture_logs = true;
		if (!incoming.report_driver.empty()) result.report_driver = incoming.report_driver;
		if (!incoming.report_connection_path.empty()) result.report_connection_path = incoming.report_connection_path;
		if (!incoming.connection_format.empty()) result.connection_format = incoming.connection_format;

		if (incoming.enterprise) result.enterprise = true;
		if (incoming.defect) result.defect = true;

		if (incoming.run_id > 0) result.run_id = incoming.run_id;
		if (!incoming.run_title.empty()) result.run_title = incoming.run_title;
		if (!incoming.run_description.empty()) result.run_description = incoming.run_description;
		if (incoming.plan_id > 0) result.plan_id = incoming.plan_id;
		if (incoming.batch_size > 0) result.batch_size = incoming.batch_size;

		return result;
	}

	void qase_reporter_finish(HttpClient& http, const QaseConfig& cfg) {
		MinimalQaseApiAdapter adapter;
		QaseApi api;
		adapter.submit_report(api, http, cfg);
	}

#ifdef QASE_REPORTER_FULL_MODE_ENABLED
	void qase_save_report(const std::vector<TestResult>& results, const std::string& path) {
		// prepare flat JSON for schema
		nlohmann::json report;
		report["results"] = nlohmann::json::array();

		for (const auto& r : results) {
			nlohmann::json entry;
			entry["title"] = !r.meta.title.empty() ? r.meta.title : r.name;
			entry["status"] = r.passed ? "passed" : "failed";

			if (r.meta.case_id > 0) {
				entry["id"] = "TC-" + std::to_string(r.meta.case_id);
			}

			for (const auto& [key, val] : r.meta.fields) {
				try {
					entry[key] = std::stod(val);
				} catch (...) {
					entry[key] = val;
				}
			}

			report["results"].push_back(entry);
		}

		std::ofstream out(path);
		if (!out) {
			throw std::runtime_error("Failed to open file for writing report");
		}
		out << report.dump(2);
		out.close();
	}
#endif


#ifdef QASE_REPORTER_FULL_MODE_ENABLED
	// FullQaseApiAdapter will go here
#else
	void MinimalQaseApiAdapter::submit_report(IQaseApi& api, HttpClient& http, const QaseConfig& cfg) {
		qase_submit_report(api, http, cfg);
	}
#endif

}
