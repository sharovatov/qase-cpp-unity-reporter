## Qase Unity C++ Reporter

This lightweight reporter collects Unity test results in any C++ environment (even on embedded platforms like ESP32) and sends them to Qase TestOps via its API.

The reporter supports:
- Simple drop-in replacement for Unity macros
- Collecting test results during execution
- Submitting results to an existing or new test run
- Marking test runs as complete (optional)
- Passing per-test metadata (case ID, title override, custom fields)

The reporter has **two** build targets, one for compute-constrained devices like ESP-32 and other embedded; and one for "bigger" systems.

When `QASE_REPORTER_FULL_MODE=ON` (see [brt](https://github.com/sharovatov/qase-cpp-unity-reporter/blob/main/brt)), the reporter is built with support for:
- schema validation
- local report saving

When `QASE_REPORTER_FULL_MODE=OFF` (see [brt](https://github.com/sharovatov/qase-cpp-unity-reporter/blob/main/brt-esp)), the reporter only supports collecting and sending test run data to Qase TestOps API.

## Basic reporter lifecycle

1. Starts the test run in Qase
2. Run the tests locally with C++ Unity
3. For each test, record the result
4. Submit all recorded results to Qase
6. Complete the test run in Qase

## External dependencies and STL usage

### External

1. **[nlohmann::json](https://github.com/nlohmann/json)**

Used for serialising and deserialising JSON in qase_serialize_results, load_qase_config_from_file, qase_submit_results, etc.

2. **[json-schema-validator](https://github.com/pboettch/json-schema-validator)**

Used to validate report json payload against json schema draft 4

3. **[yq](https://github.com/mikefarah/yq)**

Used to transform [Qase report yaml schemas](https://github.com/qase-tms/specs/tree/master/report) from YAML to JSON. This is needed to validate the report schemas.

4. **[openapi-schema-to-json-schema](https://github.com/openapi-contrib/openapi-schema-to-json-schema)**

Used to convert json from openapi schema to json schema draft 4 so that json-schema-validator can work.

### STL

- std::string — for all the strings
- std::vector — for storing test results (std::vector<TestResult>)
- std::map<std::string, std::string> — for custom fields in QaseResultMeta
- std::to_string – for serialising run ID into the request URL
- std::runtime_error – for throwing errors on config parsing and validation
- std::invalid_argument – for checking result name presence
- std::ifstream, std::ofstream – **only on desktop** of reading/writing config files

## TODO:

- [ ] Move bulk results submittion to API v2
- [ ] Support all the features from the [config file](https://github.com/qase-tms/qase-javascript/tree/main/qase-javascript-commons#configuration)
- [x] Support saving to a JSON file without submitting to Qase like [here](https://github.com/qase-tms/specs/tree/master/report)
