#include <cassert>
#include <iostream>
#include "qase_reporter.h"
#include "test_recorder.cpp"
#include "test_submitter.cpp"

#define RUN_TEST(test_func) \
    std::cout << "Running " #test_func "... "; \
    test_func(); \
    std::cout << "OK\n";

using namespace qase;

int main()
{
	RUN_TEST(test_results_accepted_stored);
	RUN_TEST(test_results_are_empty_when_none_added);
	RUN_TEST(test_result_with_empty_name_rejected);
	RUN_TEST(test_multiple_results_are_stored_correctly);
	RUN_TEST(test_results_are_serialized_to_json);
	RUN_TEST(test_start_run_returns_run_id);
	RUN_TEST(test_start_run_handles_wrong_project);
	RUN_TEST(test_submit_results_handles_wrong_project);
	RUN_TEST(test_submit_results_happy_path);
	RUN_TEST(test_complete_run_handles_wrong_project);
	RUN_TEST(test_complete_run_happy_path);
	RUN_TEST(test_start_run_calls_correct_url);
	RUN_TEST(test_start_run_sets_token_header);
	RUN_TEST(test_submit_results_sets_token_header);
	RUN_TEST(test_complete_run_sets_token_header);
	RUN_TEST(test_submit_results_passes_payload_correctly);

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
