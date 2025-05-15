#include <iostream>
#include "qase_reporter.h"

using namespace qase;

struct FakeHttpClient : public qase::HttpClient {
	std::string canned_response;
	std::string called_url;
	std::string called_payload;
	std::vector<std::string> called_headers;

	std::string post(const std::string& url, const std::string& body, const std::vector<std::string>& headers) override {
		called_url = url;
		called_headers = headers;
		called_payload = body;
		return canned_response;
	}
};

const std::string test_token = "FAKE_TOKEN_456";
const std::string empty_payload = "{ \"results\":[] }";

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

// we need to receive run_id from qase to register current run results
void test_start_run_returns_run_id()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	// qase_start_run must call HttpClient.post to retrieve the run_id from Qase API
	uint64_t run_id = qase_start_run(fake, "ET1", test_token);
	assert(run_id == 123456);
}

// when wrong project id is passed, Qase API returns errorMessage: "Project is not found."
// and there's no way for us to gracefully degrade, we must throw
void test_start_run_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found.");
	expect_qase_api_error(fake, [&]() { qase_start_run(fake, "ET1", test_token); }, "Project is not found.");
}

// when we're trying to call Qase API's bulk result method with the wrong project, there's no way to gracefully degrade, it should just throw
void test_submit_results_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found.");
	expect_qase_api_error(fake, [&]() {
			qase_submit_results(fake, "ET1", 123456, test_token, empty_payload);
			}, "Project is not found.");
}

// when Qase API responds with { "status": true }, it means that the submit_resuts worked fine!
void test_submit_results_happy_path()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	bool result = qase_submit_results(fake, "ET1", 123456, test_token, empty_payload);

	assert(result == true && "Expected qase_submit_results to return true on success");
}

// when trying to complete a run with an invalid run id or invalid project code, 
// Qase API returns an error, and we should throw
void test_complete_run_handles_wrong_project()
{
	auto fake = make_fake_with_error("Project is not found."); //when the error is Test run not found, the same logics would apply

	expect_qase_api_error(fake, [&]() {
			qase_complete_run(fake, "ET1", 123456, test_token);
			}, "Project is not found.");
}

// when Qase API responds with { "status": true }, qase_complete_run should return true
void test_complete_run_happy_path()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	bool result = qase_complete_run(fake, "ET1", 123456, test_token);

	assert(result == true && "Expected qase_complete_run to return true on success");
}

// now we need to make sure that qase_start_run correctly creates the URL
void test_start_run_calls_correct_url()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	qase_start_run(fake, "ET1", test_token);

	assert(fake.called_url == "https://api.qase.io/v1/run/ET1");
}

// helper: searches for token in the fake http client
void expect_token_header_set(FakeHttpClient& fake, const std::string& expected_token)
{
	bool token_found = false;
	for (const auto& header : fake.called_headers) {
		if (header == "Token: " + expected_token) {
			token_found = true;
			break;
		}
	}
	assert(token_found && "Expected Token header to be set correctly");
}

// qase_start_run must receive the token and pass it in the HTTP header correctly
void test_start_run_sets_token_header()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	qase_start_run(fake, "ET1", test_token);

	expect_token_header_set(fake, test_token);
}

// qase_submit_results must receive the token and pass it in the HTTP header correctly
void test_submit_results_sets_token_header()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	qase_submit_results(fake, "ET1", 123456, test_token, empty_payload);

	expect_token_header_set(fake, test_token);
}

// qase_complete_run must receive the token and pass it in the HTTP header correctly
void test_complete_run_sets_token_header()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	qase_complete_run(fake, "ET1", 123456, test_token);

	expect_token_header_set(fake, test_token);
}

// qase_submit_results must pass the payload to HTTP
void test_submit_results_passes_payload_correctly()
{
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	const std::string expected_payload = R"({
        "results": [
            { "case": { "title": "TestA" }, "status": "passed" },
            { "case": { "title": "TestB" }, "status": "failed" }
        ]
    })";

	qase_submit_results(fake, "ET1", 123456, test_token, expected_payload);

	// parse both payloads as JSON and compare them structurally
	auto expected_json = nlohmann::json::parse(expected_payload);
	auto actual_json = nlohmann::json::parse(fake.called_payload);

	assert(expected_json == actual_json && "Expected payload to be passed to HttpClient");
}

// qase_submit_report must follow this flow:
// 1. take all the results accumulated from qase_reporter_add_result calls
// 2. start test run in Qase API with qase_start_run
// 3. bulk submit all serialized results to Qase API with qase_submit_results
// 4. complete test run in Qase API with qase_complete_run
//
// within this flow, correct parameters should be passed between functions
void test_qase_submit_report_submits_results() {

	// called functions will accumulated here
	std::vector<std::string> call_sequence;

	// qase_start_run returns uint64_t id of the newly created run
	auto fake_start_run = [&]() -> uint64_t {
		call_sequence.push_back("start_run");
		return 123456;
	};

	// qase_submit_results returns true when the bulk submit is successful
	auto fake_submit_results = [&](const std::string& payload) -> bool {
		call_sequence.push_back("submit_results");

		// there must be some payload (ideally - serialised test run results)
		assert(!payload.empty() && "Expected non-empty payload to be passed to submit_results");

		return true;
	};

	// qase_complete_run returns true when the run is successfully completed
	auto fake_complete_run = [&]() -> bool {
		call_sequence.push_back("complete_run");
		return true;
	};

	qase_submit_report(fake_start_run, fake_submit_results, fake_complete_run);

	assert(
			(call_sequence == std::vector<std::string>{"start_run", "submit_results", "complete_run"}) &&
			"Expected functions to be called in correct order");
}

