## Qase Unity C++ Reporter

This lightweight reporter collects Unity test results in any C++ environment (even on embedded platforms like ESP32) and sends them to Qase TestOps via its API.

It supports:
- Collecting test results during execution
- Submitting results to an existing or new test run
- Marking test runs as complete (optional)
- Per-test metadata (case ID, title override, custom fields)

## Reporter lifecycle

1. Starts the test run in Qase
2. Run the tests locally with C++ Unity
3. For each test, record the result
4. Submit all recorded results to Qase
6. Complete the test run in Qase

## TODO:

- [x] Support test results collection — `qase_reporter_add_result`
- [x] serialize them to JSON — `qase_reporter_serialize_to_json`
- [x] start the test run via API `POST /run/{projectCode}`
- [x] submitting test results via API `POST /result/{projectCode}/{runId}/bulk`
- [x] complete the run via API `PATCH /run/{projectCode}/{runId}/complete`
- [x] Support for the file config for `QASE_API_TOKEN`, `QASE_PROJECT_CODE`
- [x] Support specifying the run id
- [x] support adding metadata to tests
- [ ] Support all other features from the [config file](https://github.com/qase-tms/qase-javascript/tree/main/qase-javascript-commons#configuration)
- [ ] Move bulk results submittion to API v2
- [ ] Support saving to a JSON file without submitting to Qase like [here](https://github.com/qase-tms/specs/tree/master/report)
