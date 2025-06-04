void test_report_mode_skips_api_calls() {
    FakeQaseApi api;
    FakeHttpClient http;

    qase_reporter_reset();
    qase_reporter_add_result("dummy", true);

    QaseConfig cfg = make_test_config();
    cfg.mode = "report";

    qase_submit_report(api, http, cfg);

    assert(api.calls.empty() && "In report mode, no API calls should be made");
}
