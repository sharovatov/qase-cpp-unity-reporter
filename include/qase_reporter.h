#pragma once

#include <string>
#include <vector>
#include <map>

namespace qase {

	struct QaseResultMeta {
		int case_id = 0;
		std::string title;
		std::map<std::string, std::string> fields;
	};

	struct TestResult {
		std::string name;
		bool passed;
		QaseResultMeta meta;
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

		int run_id = 0;

		std::string run_title;
		QaseConfig() {
			std::time_t now = std::time(nullptr);
			std::tm* tm_now = std::localtime(&now);

			char time_part[32];
			std::strftime(time_part, sizeof(time_part), "%Y-%m-%d %H:%M", tm_now);

			run_title = "Automated run ";
			run_title += time_part;
		}

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
	void qase_reporter_add_result(const std::string& name, bool passed, const QaseResultMeta& meta);
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

	struct IQaseApiAdapter {
		virtual void submit_report(IQaseApi& api, HttpClient& http, const QaseConfig& cfg) = 0;
		virtual ~IQaseApiAdapter() = default;
	};

	#ifdef QASE_REPORTER_FULL_MODE_ENABLED
	// full QaseApiAdapter will go here
	#else
	struct MinimalQaseApiAdapter : public IQaseApiAdapter {
		void submit_report(IQaseApi& api, HttpClient& http, const QaseConfig& cfg) override;
	};
	#endif


#ifndef ESP_PLATFORM
	QaseConfig load_qase_config_from_file(const std::string& path);
#endif

	QaseConfig merge_config(const QaseConfig& base, const QaseConfig& incoming);

	QaseConfig load_qase_config_from_env(const std::string& prefix);

	void qase_save_report(const std::vector<TestResult>& results, const std::string& path);

}

// this macro wrapper needs to be used to run each test instead of unity's UNITY_BEGIN
// so that the tests start from clean state
#define QASE_UNITY_BEGIN() \
	qase::qase_reporter_reset(); \
	UNITY_BEGIN();

// this macros will be chosen for QASE_RUN_TEST(func)
#define QASE_RUN_TEST_SIMPLE(test_func) \
	RUN_TEST(test_func); \
	qase_reporter_add_result(#test_func, Unity.TestFailures == Unity.CurrentTestFailed);

// this macros will be chosen for QASE_RUN_TEST(func, meta)
#define QASE_RUN_TEST_META(test_func, meta) \
	RUN_TEST(test_func); \
	qase_reporter_add_result(#test_func, Unity.TestFailures == Unity.CurrentTestFailed, meta);

#define GET_QASE_RUN_TEST_MACRO(_1, _2, NAME, ...) NAME
#define QASE_RUN_TEST(...) \
    GET_QASE_RUN_TEST_MACRO(__VA_ARGS__, QASE_RUN_TEST_META, QASE_RUN_TEST_SIMPLE)(__VA_ARGS__)

// this macro wrapper needs to be used after all the tests completion instead of unity's UNITY_END
// so that collected test results are sent to Qase API
#define QASE_UNITY_END(http_client, cfg) \
	UNITY_END(); \
	qase_reporter_finish(http_client, cfg);

/*
 *  usage examples:
 *
 *  // normal test, basic unity replacement
 *  QASE_RUN_TEST(test_wifi_connects_successfully);
 *
 *
 *
 *  // test run with additional meta parameters
 *	QaseResultMeta meta = {
 *		.case_id = 42,
 *		.title = "WiFi connects",
 *		.fields = { {"severity", "critical"} }
 *	};
 *	QASE_RUN_TEST(test_wifi_connects_successfully, meta);
 *
 * */
