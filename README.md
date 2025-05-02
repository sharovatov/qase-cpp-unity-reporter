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
