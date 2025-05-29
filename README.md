## Reporter lifecycle

1. Start the run to create a test run in Qase: call `POST /run/{projectCode}` with a run title, environment, etc. Qase will returns a `run_id` which is needed for the next step.
2. Run the tests locally in C++
3. Record test results
4. Serialise results into JSON in the format Qase expects
5. Submit all results using `POST /result/{projectCode}/{runId}/bulk`
6. Complete the run by calling `PATCH /run/{projectCode}/{runId}/complete`

## Minimal payload structure

```
{
  "results": [
    {
      "case": {
        "title": "MyFirstTest"
      },
      "status": "passed"
    },
    {
      "case": {
        "title": "MySecondTest"
      },
      "status": "failed"
    }
  ]
}
```

## TODO:

- [x] Support test results collection — `qase_reporter_add_result`
- [x] serialize them to JSON — `qase_reporter_serialize_to_json`
- [x] start the test run via API `POST /run/{projectCode}`
- [x] submitting test results via API `POST /result/{projectCode}/{runId}/bulk`
- [x] complete the run via API `PATCH /run/{projectCode}/{runId}/complete`
- [x] Support for the file config for `QASE_API_TOKEN`, `QASE_PROJECT_CODE`
- [ ] Support all other features from the [config file](https://github.com/qase-tms/qase-javascript/tree/main/qase-javascript-commons#configuration)
- [ ] Move bulk results submittion to API v2
- [ ] Support saving to a JSON file without submitting to Qase like [here](https://github.com/qase-tms/specs/tree/master/report)
- [ ] Support specifying the run id
- [ ] support adding metadata to tests
