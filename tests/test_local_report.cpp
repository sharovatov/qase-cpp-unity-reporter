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

void test_report_mode_writes_payload_to_file() {
    qase_reporter_reset();

    qase_reporter_add_result("TestThatPasses", true);

    QaseConfig cfg;
    cfg.mode = "report";
    cfg.report_connection_path = "test_report_output.json";

    FakeQaseApi api;
    FakeHttpClient http;

    qase_submit_report(api, http, cfg);

    // check the file is written
    std::ifstream in("test_report_output.json");
    assert(in.good() && "Expected output file to be created in report mode");

    nlohmann::json json_payload;
    in >> json_payload;

    assert(json_payload.contains("results"));
    assert(json_payload["results"].is_array());
    assert(json_payload["results"].size() == 1);
    assert(json_payload["results"][0]["case"]["title"] == "TestThatPasses");
    assert(json_payload["results"][0]["status"] == "passed");

    in.close();
    std::remove("test_report_output.json");
}
