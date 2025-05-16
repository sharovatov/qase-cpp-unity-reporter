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

	std::string qase_reporter_serialize_to_json();

	uint64_t qase_start_run(HttpClient& http, const std::string& project_code, const std::string& token);
	bool qase_submit_results(HttpClient& http, const std::string& project_code, uint64_t run_id, const std::string& token, const std::string& payload);
	bool qase_complete_run(HttpClient& http, const std::string& project_code, uint64_t run_id, const std::string& token);

	struct IQaseApi {
		virtual uint64_t qase_start_run(HttpClient&, const std::string&, const std::string&) = 0;
		virtual bool qase_submit_results(HttpClient&, const std::string&, uint64_t, const std::string&, const std::string&) = 0;
		virtual bool qase_complete_run(HttpClient&, const std::string&, uint64_t, const std::string&) = 0;
		virtual ~IQaseApi() = default;
	};

	void qase_submit_report(
			IQaseApi& api,
			HttpClient& http,
			const std::string& project_code,
			const std::string& token
		);

}
