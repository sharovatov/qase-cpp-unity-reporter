#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

void test_results_accepted_stored()
{
	// qase reporter should be able to accept test execution result and store it
	qase_reporter_add_result("MyFirstTest", true);

	const auto& results = qase_reporter_get_results();
	assert(results.size() == 1);
	assert(results[0].name == "MyFirstTest");
	assert(results[0].passed == true);

}

int main()
{

	test_results_accepted_stored();

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
