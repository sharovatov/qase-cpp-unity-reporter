#include <cassert>
#include <iostream>
#include "qase_reporter.h"

using namespace qase;

struct FakeHttpClient : public qase::HttpClient {
	std::string canned_response;
	std::string called_url;
	std::vector<std::string> called_headers;

	std::string post(const std::string& url, const std::string&, const std::vector<std::string>& headers) override {
		called_url = url;
		called_headers = headers;
		return canned_response;
	}
};

template<typename Func>
void expect_qase_api_error(FakeHttpClient& fake, Func api_call, const std::string& expected_message)
{
	bool exception_thrown = false;
	try {
		api_call();
	} catch (const std::runtime_error& e) {
		exception_thrown = std::string(e.what()).find(expected_message) != std::string::npos;
	}

	assert(exception_thrown && ("Expected std::runtime_error with message: " + expected_message).c_str());
}

FakeHttpClient make_fake_with_error(const std::string& error_message) {
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": false, "errorMessage": ")" + error_message + R"(" })";
	return fake;
}

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

// qase reporter serialises accumulated test results to json
void test_results_are_serialized_to_json()
{
	qase_reporter_reset();
	qase_reporter_add_result("MyFirstTest", true);
	qase_reporter_add_result("SecondTest", false);

	std::string json = qase_reporter_serialize_to_json();

	assert(json.find("\"results\"") != std::string::npos);
	assert(json.find("\"title\":\"MyFirstTest\"") != std::string::npos);
	assert(json.find("\"status\":\"passed\"") != std::string::npos);
	assert(json.find("\"title\":\"SecondTest\"") != std::string::npos);
	assert(json.find("\"status\":\"failed\"") != std::string::npos);
}

// we need to receive run_id from qase to register current run results
void test_start_run_returns_run_id()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	// qase_start_run must call HttpClient.post to retrieve the run_id from Qase API
	uint64_t run_id = qase_start_run(fake, "ET1", "fake token");
	assert(run_id == 123456);
}

// when wrong project id is passed, Qase API returns errorMessage: "Project is not found."
// and there's no way for us to gracefully degrade, we must throw
void test_start_run_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found.");
	expect_qase_api_error(fake, [&]() { qase_start_run(fake, "ET1", "fake token"); }, "Project is not found.");
}

// when we're trying to call Qase API's bulk result method with the wrong project, there's no way to gracefully degrade, it should just throw
void test_submit_results_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found.");
	expect_qase_api_error(fake, [&]() { qase_submit_results(fake, "ET1", 123456); }, "Project is not found.");
}

// when Qase API responds with { "status": true }, it means that the submit_resuts worked fine!
void test_submit_results_happy_path()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	bool result = qase_submit_results(fake, "ET1", 123456);

	assert(result == true && "Expected qase_submit_results to return true on success");
}

// when trying to complete a run with an invalid run id or invalid project code, 
// Qase API returns an error, and we should throw
void test_complete_run_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found."); //when the error is Test run not found, the same logics would apply

	expect_qase_api_error(fake, [&]() { qase_complete_run(fake, "ET1", 123456); }, "Project is not found.");
}

// when Qase API responds with { "status": true }, qase_complete_run should return true
void test_complete_run_happy_path()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	bool result = qase_complete_run(fake, "ET1", 123456);

	assert(result == true && "Expected qase_complete_run to return true on success");
}

// now we need to make sure that qase_start_run correctly creates the URL
void test_start_run_calls_correct_url()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	qase_start_run(fake, "ET1", "fake token");

	assert(fake.called_url == "https://api.qase.io/v1/run/ET1");
}

// qase_start_run must receive the token and pass it in the HTTP header correctly
void test_start_run_sets_token_header()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	const std::string expected_token = "FAKE_TOKEN_456";

	qase_start_run(fake, "ET1", expected_token);

	bool token_found = false;
	for (const auto& header : fake.called_headers) {
		if (header == "Token: " + expected_token) {
			token_found = true;
			break;
		}
	}
	assert(token_found && "Expected Token header to be set correctly");
}

int main()
{
	test_results_accepted_stored();
	test_results_are_empty_when_none_added();
	test_result_with_empty_name_rejected();
	test_multiple_results_are_stored_correctly();
	test_results_are_serialized_to_json();
	test_start_run_returns_run_id();
	test_start_run_handles_wrong_project();
	test_submit_results_handles_wrong_project();
	test_submit_results_happy_path();
	test_complete_run_handles_wrong_project();
	test_complete_run_happy_path();
	test_start_run_calls_correct_url();
	test_start_run_sets_token_header();

	std::cout << "All TDD checks passed!" << std::endl;

	return 0;
}
