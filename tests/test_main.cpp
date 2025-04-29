#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

int main()
{
	// qase reporter should be able to accept test execution result
	qase_reporter_add_result("MyFirstTest", true);

	// qase reporter should also be able to store the test execution result
	const auto& results = qase_reporter_get_results();
	assert(results.size() == 1);
    assert(results[0].name == "StoreTest");
    assert(results[0].passed == true);

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
