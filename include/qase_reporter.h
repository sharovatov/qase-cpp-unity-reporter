#pragma once

#include <string>
#include <vector>

namespace qase {

	struct TestResult {
		std::string name;
		bool passed;
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
		virtual uint64_t qase_start_run(HttpClient&, const std::string&, const std::string&) = 0;
		virtual bool qase_submit_results(HttpClient&, const std::string&, uint64_t, const std::string&, const std::string&) = 0;
		virtual bool qase_complete_run(HttpClient&, const std::string&, uint64_t, const std::string&) = 0;
		virtual ~IQaseApi() = default;
	};

	struct QaseApi : public IQaseApi {
		uint64_t qase_start_run(HttpClient&, const std::string&, const std::string&) override;
		bool qase_submit_results(HttpClient&, const std::string&, uint64_t, const std::string&, const std::string&) override;
		bool qase_complete_run(HttpClient&, const std::string&, uint64_t, const std::string&) override;
	};

	struct QaseConfig {
		std::string token;
		std::string host;
		std::string project;
	};

	void qase_submit_report(
			IQaseApi& api,
			HttpClient& http,
			const QaseConfig& cfg
		);

	QaseConfig load_qase_config(const std::string& path);
}
