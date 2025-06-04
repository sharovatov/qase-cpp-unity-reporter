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

## External dependencies and STL usage

### External

1. **[nlohmann::json](https://github.com/nlohmann/json)**

Used for serialising and deserialising JSON in qase_serialize_results, load_qase_config_from_file, qase_submit_results, etc.

2. **[yq](https://github.com/mikefarah/yq)**

Used to transform [Qase report yaml schemas](https://github.com/qase-tms/specs/tree/master/report) from YAML to JSON. This is needed to validate the report schemas.

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
- [ ] Support saving to a JSON file without submitting to Qase like [here](https://github.com/qase-tms/specs/tree/master/report) — **should only be done for desktop**
