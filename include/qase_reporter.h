#pragma once

#include <string>
#include <vector>

namespace qase {

	struct TestResult {
		std::string name;
		bool passed;
	};

	void qase_reporter_add_result(const std::string& name, bool passed);
	const std::vector<TestResult>& qase_reporter_get_results();

	void qase_reporter_reset();
}
