#include <iostream>
#include <cassert>
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


QaseConfig make_test_config() {
	QaseConfig cfg;
	cfg.project = "ET1";
	cfg.token = test_token;
	cfg.host = "api.qase.io";
	return cfg;
}


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
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	QaseConfig cfg = make_test_config();

	// qase_start_run must call HttpClient.post to retrieve the run_id from Qase API
	uint64_t run_id = api.qase_start_run(fake, cfg);
	assert(run_id == 123456);
}

// when wrong project id is passed, Qase API returns errorMessage: "Project is not found."
// and there's no way for us to gracefully degrade, we must throw
void test_start_run_handles_wrong_project()
{
	QaseApi api;
	auto fake = make_fake_with_error("Project is not found.");
	QaseConfig cfg = make_test_config();

	expect_qase_api_error(fake, [&]() { api.qase_start_run(fake, cfg); }, "Project is not found.");
}

// when we're trying to call Qase API's bulk result method with the wrong project, there's no way to gracefully degrade, it should just throw
void test_submit_results_handles_wrong_project()
{
	QaseApi api;
	auto fake = make_fake_with_error("Project is not found.");
	QaseConfig cfg = make_test_config();
	expect_qase_api_error(fake, [&]() {
			api.qase_submit_results(fake, cfg, 123456, empty_payload);
			}, "Project is not found.");
}

// when Qase API responds with { "status": true }, it means that the submit_resuts worked fine!
void test_submit_results_happy_path()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	QaseConfig cfg = make_test_config();

	bool result = api.qase_submit_results(fake, cfg, 123456, empty_payload);

	assert(result == true && "Expected qase_submit_results to return true on success");
}

// when trying to complete a run with an invalid run id or invalid project code, 
// Qase API returns an error, and we should throw
void test_complete_run_handles_wrong_project()
{
	QaseApi api;

	auto fake = make_fake_with_error("Project is not found."); //when the error is Test run not found, the same logics would apply

	QaseConfig cfg = make_test_config();

	expect_qase_api_error(fake, [&]() {
			api.qase_complete_run(fake, cfg, 123456);
			}, "Project is not found.");
}

// when Qase API responds with { "status": true }, qase_complete_run should return true
void test_complete_run_happy_path()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	QaseConfig cfg = make_test_config();

	bool result = api.qase_complete_run(fake, cfg, 123456);

	assert(result == true && "Expected qase_complete_run to return true on success");
}

// now we need to make sure that qase_start_run correctly creates the URL
void test_start_run_calls_correct_url()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	QaseConfig cfg = make_test_config();

	api.qase_start_run(fake, cfg);

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
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	QaseConfig cfg = make_test_config();

	api.qase_start_run(fake, cfg);

	expect_token_header_set(fake, test_token);
}

// qase_submit_results must receive the token and pass it in the HTTP header correctly
void test_submit_results_sets_token_header()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	QaseConfig cfg = make_test_config();

	api.qase_submit_results(fake, cfg, 123456, empty_payload);

	expect_token_header_set(fake, test_token);
}

// qase_complete_run must receive the token and pass it in the HTTP header correctly
void test_complete_run_sets_token_header()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	QaseConfig cfg = make_test_config();

	api.qase_complete_run(fake, cfg, 123456);

	expect_token_header_set(fake, test_token);
}

// qase_submit_results must pass the payload to HTTP
void test_submit_results_passes_payload_correctly()
{
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true })";

	const std::string expected_payload = R"({
        "results": [
            { "case": { "title": "TestA" }, "status": "passed" },
            { "case": { "title": "TestB" }, "status": "failed" }
        ]
    })";

	QaseConfig cfg = make_test_config();

	api.qase_submit_results(fake, cfg, 123456, expected_payload);

	// parse both payloads as JSON and compare them structurally
	auto expected_json = nlohmann::json::parse(expected_payload);
	auto actual_json = nlohmann::json::parse(fake.called_payload);

	assert(expected_json == actual_json && "Expected payload to be passed to HttpClient");
}

// very simple fake for the Qase Api, this allows to check the orchestration flow is correct in 
// qase_submit_report
struct FakeQaseApi : public IQaseApi {
	std::vector<std::string> calls;

	// captured arguments
	std::string start_project_code;
	std::string start_token;

	uint64_t submit_run_id = 0;
	std::string submit_payload;

	uint64_t complete_run_id = 0;

	uint64_t qase_start_run(HttpClient&, const QaseConfig& cfg) override {
		calls.push_back("start");
		start_project_code = cfg.project;
		start_token = cfg.token;
		return 42;
	}

	bool qase_submit_results(HttpClient&, const QaseConfig& cfg, uint64_t run_id, const std::string& payload) override {
		calls.push_back("submit");
		submit_run_id = run_id;
		submit_payload = payload;
		return true;
	}

	bool qase_complete_run(HttpClient&, const QaseConfig&, uint64_t run_id) override {
		calls.push_back("complete");
		complete_run_id = run_id;
		return true;
	}
};

// qase_submit_report must follow this flow:
// 1. take all the results accumulated from qase_reporter_add_result calls
// 2. if no run_id is set, start new test run in Qase API with qase_start_run
// 3. bulk submit all serialized results to Qase API with qase_submit_results
// 4. complete test run in Qase API with qase_complete_run
//
// also all the parameters passing is checked
void test_orchestrator_uses_iqaseapi_flow() {
	FakeQaseApi api;
	FakeHttpClient http;

	qase_reporter_reset();
	qase_reporter_add_result("dummy", true);

	QaseConfig cfg = make_test_config();
	cfg.run_id = 0; // explicitly no run_id

	qase_submit_report(api, http, cfg);

	// expected call sequence
	assert((api.calls == std::vector<std::string>{"start", "submit", "complete"}));

	// verify arguments passed correctly
	assert(api.start_project_code == cfg.project);
	assert(api.start_token == cfg.token);
	assert(api.submit_run_id == 42);
	assert(!api.submit_payload.empty());
	assert(api.complete_run_id == 42);
}

// no results recorded — qase_submit_report should do nothing
void test_orchestrator_does_nothing_if_no_results() {
	FakeQaseApi api;
	FakeHttpClient http;

	qase_reporter_reset();

	QaseConfig cfg = make_test_config();

	qase_submit_report(api, http, cfg);

	// Expect no API calls to have happened
	assert(api.calls.empty() && "Expected no API calls if no results are present");
}

// if run_id is passed from the config, we mustn't call qase_start_run
// and just pass the provided run_id to qase_submit_results
void test_orchestrator_skips_start_run_if_run_id_provided() {
	FakeQaseApi api;
	FakeHttpClient http;

	qase_reporter_reset();
	qase_reporter_add_result("dummy", true);

	QaseConfig cfg = make_test_config();
	cfg.run_id = 99; // simulate a pre-existing run

	qase_submit_report(api, http, cfg);

	// start must be skipped
	assert((api.calls == std::vector<std::string>{"submit", "complete"}));

	// verify passed run_id is used directly
	assert(api.submit_run_id == 99);
	assert(!api.submit_payload.empty());
	assert(api.complete_run_id == 99);
}

void test_start_run_uses_run_title_if_provided() {
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123 } })";

	QaseConfig cfg = make_test_config();
	cfg.run_title = "Custom Test Run 12345";

	api.qase_start_run(fake, cfg);

	// the payload must include our passed custom run title
	auto json_payload = nlohmann::json::parse(fake.called_payload);

	assert(json_payload.contains("title") && "Expected payload to contain 'title'");
	assert(json_payload["title"] == "Custom Test Run 12345");
}

void test_start_run_sets_description_if_present() {
	QaseApi api;
	FakeHttpClient fake;
	fake.canned_response = R"({ "status": true, "result": { "id": 123456 } })";

	QaseConfig cfg = make_test_config();
	cfg.run_description = "Run triggered from CI pipeline";

	api.qase_start_run(fake, cfg);

	// make sure description is in the payload
	auto json = nlohmann::json::parse(fake.called_payload);

	assert(json["description"] == cfg.run_description);
}

// if the config file says "don't complete the run", we obey
void test_orchestrator_skips_complete_run_if_config_false() {
	FakeQaseApi api;
	FakeHttpClient http;

	qase_reporter_reset();
	qase_reporter_add_result("dummy", true);

	QaseConfig cfg = make_test_config();
	cfg.run_complete = false;

	qase_submit_report(api, http, cfg);

	// verify that only "start" and "submit" were called
	assert((api.calls == std::vector<std::string>{"start", "submit"}));
}
