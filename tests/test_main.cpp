#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

int main()
{

	// qase reporter should be able to accept test execution result
	qase_reporter_add_result("MyFirstTest", true);

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
