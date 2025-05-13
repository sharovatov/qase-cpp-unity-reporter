#include <nlohmann/json.hpp>
#include "qase_reporter.h"

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

	std::string qase_reporter_serialize_to_json() {
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

	// qase_start_run should call Qase API and return new test run
	uint64_t qase_start_run(HttpClient& http, const std::string& project_code) {
		const std::string url = "https://api.qase.io/v1/run/" + project_code;
		const std::string payload = R"({ "title": "Unity Test Run", "include_all_cases": true })";
		const std::vector<std::string> headers = {
			"accept: application/json",
			"content-type: application/json",
			"Token: 4a02e17acfa32e7b71067e3beb597490f8a9bda427697c2a3bf49044582ee668"
		};

		std::string response = http.post(url, payload, headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		// extract result.id if present
		if (json.contains("result") && json["result"].contains("id")) {
			return json["result"]["id"].get<uint64_t>();
		}

		throw std::runtime_error("qase_start_run unknown error");
	}

	bool qase_submit_results(HttpClient& http, const std::string& project_code, uint64_t run_id) {
		const std::string url = "https://api.qase.io/v1/result/" + project_code + "/" + std::to_string(run_id) + "/bulk";

		const std::string payload = R"({ "results": [] })"; // no need for payload for now

		const std::vector<std::string> headers = {
			"accept: application/json",
			"content-type: application/json",
			"Token: 4a02e17acfa32e7b71067e3beb597490f8a9bda427697c2a3bf49044582ee668"
		};

		std::string response = http.post(url, payload, headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

		return json.contains("status") && json["status"] == true;

	}

	bool qase_complete_run(HttpClient& http, const std::string& project_code, uint64_t run_id) {

		const std::string url = "https://api.qase.io/v1/run/" + project_code + "/" + std::to_string(run_id) + "/complete";

		const std::vector<std::string> headers = {
			"accept: application/json",
			"Token: 4a02e17acfa32e7b71067e3beb597490f8a9bda427697c2a3bf49044582ee668"
		};

		std::string response = http.post(url, "", headers);
		auto json = nlohmann::json::parse(response);

		check_qase_api_error(json);

	}
}
