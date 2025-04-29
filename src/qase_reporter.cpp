#include "qase_reporter.h"

namespace qase {

	static std::vector<TestResult> collected;

	void qase_reporter_add_result(const std::string& name, bool passed) {
		collected.push_back(TestResult{name, passed});
	}

	const std::vector<TestResult>& qase_reporter_get_results() {
		return collected;
	}

	void qase_reporter_reset() {
		collected.clear();
	}

}
