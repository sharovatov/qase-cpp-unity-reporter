#include <iostream>
#include <cassert>
#include <fstream>
#include "qase_reporter.h"

using namespace qase;

// moving slowly to using the openapi client, now introducing manual adapter which
// will wrap around qase_serialize_results and qase_submit_report
#ifdef QASE_REPORTER_FULL_MODE_ENABLED
void test_adapter_submits_via_full_flow()
{

}
// tests for full QaseApiAdapter will go here
#else
void test_adapter_submits_via_minimal_flow()
{
	qase_reporter_reset();
	qase_reporter_add_result("adapter test", true);

	FakeQaseApi api;
	FakeHttpClient http;
	QaseConfig cfg = make_test_config();

	MinimalQaseApiAdapter adapter;
	adapter.submit_report(api, http, cfg);

	assert((api.calls == std::vector<std::string>{"start", "submit", "complete"}));
	assert(api.submit_run_id == 42);
	assert(!api.submit_payload.empty());
}
#endif
