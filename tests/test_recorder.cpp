#include <iostream>
#include <nlohmann/json.hpp>

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

// qase reporter serialises accumulated test results to json
void test_results_are_serialized_to_json()
{
	std::vector<TestResult> results;

	// 1. basic result without metadata
	results.push_back({ "MyFirstTest", true });

	// 2. result with metadata
	QaseResultMeta meta;
	meta.case_id = 123;
	meta.title = "Replaced Test Name";
	meta.fields["priority"] = "high";

	results.push_back({ "SecondTest", false, meta });

	// serialize
	std::string json_str = qase_serialize_results(results);

	auto parsed = nlohmann::json::parse(json_str);

	assert(parsed.contains("results"));
	assert(parsed["results"].is_array());
	assert(parsed["results"].size() == 2);

	// check first run result
	auto r1 = parsed["results"][0];
	assert(r1["status"] == "passed");
	assert(r1["case"]["title"] == "MyFirstTest");
	assert(!r1["case"].contains("case_id"));
	assert(!r1["case"].contains("priority"));

	// check second run result
	auto r2 = parsed["results"][1];
	assert(r2["status"] == "failed");
	assert(r2["case"]["title"] == "Replaced Test Name");
	assert(r2["case"]["case_id"] == 123);
	assert(r2["case"]["priority"] == "high");
}

void test_qase_reporter_add_result_accepts_meta() {
	qase_reporter_reset();

	QaseResultMeta meta;
	meta.case_id = 123;
	meta.title = "Test title with metadata";
	meta.fields["priority"] = "high";
	meta.fields["layer"] = "unit";

	qase_reporter_add_result("test_with_meta", true, meta);

	const auto& results = qase_reporter_get_results();
	assert(results.size() == 1);
	assert(results[0].name == "test_with_meta");
	assert(results[0].passed == true);
	assert(results[0].meta.case_id == 123);
	assert(results[0].meta.title == "Test title with metadata");
	assert(results[0].meta.fields.at("priority") == "high");
	assert(results[0].meta.fields.at("layer") == "unit");
}
