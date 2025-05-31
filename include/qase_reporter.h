#pragma once

#include <string>
#include <vector>

namespace qase {

	struct TestResult {
		std::string name;
		bool passed;
	};

	struct QaseConfig {
		std::string token;
		std::string host = "api.qase.io";
		std::string project;
		bool run_complete = true;

		std::string mode = "testops";
		std::string fallback = "off";
		std::string environment;
		std::string root_suite;
		bool debug = false;
		bool capture_logs = false;
		std::string report_driver = "local";
		std::string report_connection_path;
		std::string connection_format = "json";
		bool enterprise = false;
		
		// todo: support passing run_id
		int run_id;

		// todo: support adding date and time to this so it's "Automated run <Current date and time>"
		std::string run_title = "Automated run ????";

		std::string run_description = "Unity automated run";
		int plan_id;
		int batch_size = 200;

		// todo: support this
		bool defect = false;

	};

	struct ConfigResolutionInput {
		std::optional<QaseConfig> preset;
		std::optional<std::string> env_prefix;
		std::optional<std::string> file;
	};

struct HttpClient {
		virtual std::string post(const std::string& url, const std::string& body, const std::vector<std::string>& headers) = 0;

		virtual ~HttpClient() = default;
	};

	void qase_reporter_add_result(const std::string& name, bool passed);
	const std::vector<TestResult>& qase_reporter_get_results();

	void qase_reporter_reset();

	std::string qase_serialize_results(const std::vector<TestResult>& results);

	struct IQaseApi {
		virtual uint64_t qase_start_run(HttpClient&, const QaseConfig&) = 0;
		virtual bool qase_submit_results(HttpClient&, const QaseConfig&, uint64_t, const std::string&) = 0;
		virtual bool qase_complete_run(HttpClient&, const QaseConfig&, uint64_t) = 0;
		virtual ~IQaseApi() = default;
	};

	struct QaseApi : public IQaseApi {
		uint64_t qase_start_run(HttpClient&, const QaseConfig& cfg) override;
		bool qase_submit_results(HttpClient&, const QaseConfig&, uint64_t, const std::string&) override;
		bool qase_complete_run(HttpClient&, const QaseConfig&, uint64_t) override;
	};

	void qase_submit_report(
			IQaseApi& api,
			HttpClient& http,
			const QaseConfig& cfg
		);

	QaseConfig resolve_config(const ConfigResolutionInput& input);

#ifndef ESP_PLATFORM
	QaseConfig load_qase_config_from_file(const std::string& path);
#endif

	QaseConfig merge_config(const QaseConfig& base, const QaseConfig& incoming);

	QaseConfig load_qase_config_from_env(const std::string& prefix);

}
