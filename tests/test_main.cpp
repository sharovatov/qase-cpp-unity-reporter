#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

// qase reporter should be able to accept test execution result and store it
void test_results_accepted_stored()
{
	qase_reporter_add_result("MyFirstTest", true);

	const auto& results = qase_reporter_get_results();
	assert(results.size() == 1);
	assert(results[0].name == "MyFirstTest");
	assert(results[0].passed == true);

}

// if nothing is added to the reporter, it doesn't die but returns empty vector
void test_results_are_empty_when_none_added() {
    const auto& results = qase_reporter_get_results();
    assert(results.empty());
}


int main()
{
	test_results_accepted_stored();
	test_results_are_empty_when_none_added();


	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
