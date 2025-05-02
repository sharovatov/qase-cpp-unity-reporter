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

- [x] Test results collection — `qase_reporter_add_result`
- [x] JSON serialization — `qase_reporter_serialize_to_json`
- [ ] Support for the file config for `QASE_API_TOKEN`, `QASE_PROJECT_CODE`
- [ ] Starting the test run via `POST /run/{projectCode}`
- [ ] Submitting results to Qase via `POST /result/{projectCode}/{runId}/bulk`
- [ ] Completing the run via `PATCH /run/{projectCode}/{runId}/complete`
