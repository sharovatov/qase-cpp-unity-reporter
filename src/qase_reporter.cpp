#include <nlohmann/json.hpp>
#include "qase_reporter.h"
#include <fstream> // maybe we should include it only when built with fs support?

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
		if (name.empty()) {
			throw std::invalid_argument("Test name must not be empty");
		}
		collected.push_back(TestResult{name, passed});
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
			entry["case"] = { {"title", result.name} };
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

	// Qase API url
	const std::string api_url = "https://api.qase.io/v1/";


	// qase_start_run should call Qase API and return new test run
	uint64_t QaseApi::qase_start_run(HttpClient& http, const std::string& project_code, const std::string& token) {
		const std::string url = api_url + "run/" + project_code;
		const std::string payload = R"({ "title": "Unity Test Run", "include_all_cases": true })";
		const auto headers = make_headers(token);

		std::string response = http.post(url, payload, headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		// extract result.id if present
		if (json.contains("result") && json["result"].contains("id")) {
			return json["result"]["id"].get<uint64_t>();
		}

		throw std::runtime_error("qase_start_run unknown error");
	}

	bool QaseApi::qase_submit_results(HttpClient& http, const std::string& project_code, uint64_t run_id, const std::string& token, const std::string& payload) {
		const std::string url = api_url + "result/" + project_code + "/" + std::to_string(run_id) + "/bulk";
		const auto headers = make_headers(token);

		std::string response = http.post(url, payload, headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		return json.contains("status") && json["status"] == true;

	}

	bool QaseApi::qase_complete_run(HttpClient& http, const std::string& project_code, uint64_t run_id, const std::string& token) {

		const std::string url = api_url + "run/" + project_code + "/" + std::to_string(run_id) + "/complete";

		const auto headers = make_headers(token);

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

		const auto& project_code = cfg.project;
		const auto& token = cfg.token;

		// step 0: take all the results accumulated from qase_reporter_add_result calls
		const auto& results = qase_reporter_get_results();
		if (results.empty()) {
			return; // nothing to submit, skip orchestration
		}

		// step 1: take all serialised results accumulated from qase_reporter_add_result calls
		const std::string payload = qase_serialize_results(results);

		// step 2: start test run in Qase API with qase_start_run
		uint64_t run_id = api.qase_start_run(http, project_code, token);

		// step 3: bulk submit all serialized results to Qase API with qase_submit_results
		bool bulk_result = api.qase_submit_results(http, project_code, run_id, token, payload);

		// step 4: complete test run in Qase API with qase_complete_run
		bool completed = api.qase_complete_run(http, project_code, run_id, token);

	}

	QaseConfig load_qase_config(const std::string& path) {
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

		// testops.project, testops.api.token and testops.api.host must be readable
		if (!j.contains("testops") || !j["testops"].contains("api") || 
				!j["testops"]["api"].contains("token") ||
				!j["testops"]["api"].contains("host") ||
				!j["testops"].contains("project")) {
			throw std::runtime_error("Missing required field(s) in config");
		}

		QaseConfig cfg;
		cfg.token = j["testops"]["api"]["token"].get<std::string>();
		cfg.host = j["testops"]["api"]["host"].get<std::string>();
		cfg.project = j["testops"]["project"].get<std::string>();

		if (cfg.token.empty() || cfg.host.empty() || cfg.project.empty()) {
			throw std::runtime_error("Config fields must not be empty");
		}

		return cfg;
	}

}
