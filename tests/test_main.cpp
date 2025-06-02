#include <cassert>
#include <iostream>
#include "qase_reporter.h"
#include "test_recorder.cpp"
#include "test_submitter.cpp"
#include "test_config.cpp"

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
	RUN_TEST(test_orchestrator_uses_iqaseapi_flow);
	RUN_TEST(test_orchestrator_does_nothing_if_no_results);
	RUN_TEST(test_load_qase_config_parses_fields_correctly);
	RUN_TEST(test_load_qase_config_throws_if_file_missing);
	RUN_TEST(test_load_qase_config_throws_on_invalid_json);
	RUN_TEST(test_load_qase_config_throws_on_missing_fields);
	RUN_TEST(test_load_qase_config_throws_on_empty_fields);
	RUN_TEST(test_resolve_config_uses_file_if_nothing_else);
	RUN_TEST(test_resolve_config_env_vars_override_file);
	RUN_TEST(test_resolve_config_preset_overrides_env_and_file);
	RUN_TEST(test_merge_config_overrides_strings);
	RUN_TEST(test_load_qase_config_from_env_reads_expected_fields);
	RUN_TEST(test_orchestrator_skips_start_run_if_run_id_provided);
	RUN_TEST(test_start_run_uses_run_title_if_provided);
	RUN_TEST(test_default_run_title_contains_date_and_time);
	RUN_TEST(test_start_run_sets_description_if_present);
	RUN_TEST(test_load_qase_config_parses_run_complete);
	RUN_TEST(test_orchestrator_skips_complete_run_if_config_false);

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
