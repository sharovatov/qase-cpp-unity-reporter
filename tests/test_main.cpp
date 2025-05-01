#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

// qase reporter should be able to accept test execution result and store it
void test_results_accepted_stored()
{
	qase_reporter_reset();
	qase_reporter_add_result("MyFirstTest", true);

	const auto& results = qase_reporter_get_results();
	assert(results.size() == 1);
	assert(results[0].name == "MyFirstTest");
	assert(results[0].passed == true);

}

// if nothing is added to the reporter, it doesn't die but returns empty vector
void test_results_are_empty_when_none_added() 
{
	qase_reporter_reset();
	const auto& results = qase_reporter_get_results();
	assert(results.empty());
}

// test case must have a name — there are no tests without a name
void test_result_with_empty_name_rejected() 
{
	qase_reporter_reset();

	bool exception_thrown = false;

	try {
		qase_reporter_add_result("", true);
	} catch (...) 
	{
		exception_thrown = true;
	}

	assert(exception_thrown && "Expected exception for an empty test name");
}

// qase reporter accumulates multiple test results in insertion order
void test_multiple_results_are_stored_correctly()
{
	qase_reporter_reset();

	qase_reporter_add_result("TestA", true);
	qase_reporter_add_result("TestB", false);
	qase_reporter_add_result("TestC", true);

	const auto& results = qase_reporter_get_results();
	assert(results.size() == 3);

	assert(results[0].name == "TestA");
	assert(results[0].passed == true);

	assert(results[1].name == "TestB");
	assert(results[1].passed == false);

	assert(results[2].name == "TestC");
	assert(results[2].passed == true);
}

int main()
{
	test_results_accepted_stored();
	test_results_are_empty_when_none_added();
	test_result_with_empty_name_rejected();

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
