All configuration options are listed in the table below:

### Common

| Supported | Description | Config file | Environment variable | Default value | Required | Possible values |
|-----------|-------------|-------------|----------------------|----------------|----------|------------------|
|  No       | Mode of reporter                                                                                                      | `mode`                     | `QASE_MODE`                     | `off`                                  | No       | `testops`, `report`, `off` |
|  No       | Fallback mode of reporter                                                                                             | `fallback`                 | `QASE_FALLBACK`                 | `off`                                   | No       | `testops`, `report`, `off` |
| No        | Environment                                                                                                           | `environment`              | `QASE_ENVIRONMENT`              | undefined                              | No       | Any string                 |
| No        | Root suite                                                                                                            | `rootSuite`                | `QASE_ROOT_SUITE`               | undefined                               | No       | Any string                 |
| No        | Enable debug logs                                                                                                     | `debug`                    | `QASE_DEBUG`                    | `False`                                 | No       | `True`, `False`            |
| No        | Enable capture logs from `stdout` and `stderr`                                                                        | `testops.defect`           | `QASE_CAPTURE_LOGS`             | `False`                                 | No       | `True`, `False`            |

### Qase Report configuration

| Supported | Description | Config file | Environment variable | Default value | Required | Possible values |
|-----------|-------------|-------------|----------------------|----------------|----------|------------------|
| No        | Driver used for report mode                                                                                           | `report.driver`            | `QASE_REPORT_DRIVER`            | `local`                                 | No       | `local`                    |
| No        | Path to save the report                                                                                               | `report.connection.path`   | `QASE_REPORT_CONNECTION_PATH`   | `./build/qase-report`                   |          |                            |
| No        | Local report format                                                                                                   | `report.connection.format` | `QASE_REPORT_CONNECTION_FORMAT` | `json`                                  |          | `json`, `jsonp`            |

### Qase TestOps configuration

| Supported | Description | Config file | Environment variable | Default value | Required | Possible values |
|-----------|-------------|-------------|----------------------|----------------|----------|------------------|
| Yes       | Token for [API access](https://developers.qase.io/#authentication)                                                    | `testops.api.token`        | `QASE_TESTOPS_API_TOKEN`        |  undefined                              | Yes      | Any string                 |
| Yes       | Qase API host. For enterprise users, specify address: `example.qase.io`                                           | `testops.api.host`         | `QASE_TESTOPS_API_HOST`         | `qase.io`                               | No       | Any string                 |
| No        | Qase enterprise environment                                                                                           | `testops.api.enterprise`   | `QASE_TESTOPS_API_ENTERPRISE`   | `False`                                 | No       | `True`, `False`            |
| Yes       | Code of your project, which you can take from the URL: `https://app.qase.io/project/DEMOTR` - `DEMOTR` is the project code | `testops.project`          | `QASE_TESTOPS_PROJECT`          |  undefined                              | Yes      | Any string                 |
| Yes       | Qase test run ID                                                                                                      | `testops.run.id`           | `QASE_TESTOPS_RUN_ID`           |  undefined                              | No       | Any integer                |
| Yes       | Qase test run title                                                                                                   | `testops.run.title`        | `QASE_TESTOPS_RUN_TITLE`        | `Automated run <Current date and time>` | No       | Any string                 |
| Yes       | Qase test run description                                                                                             | `testops.run.description`  | `QASE_TESTOPS_RUN_DESCRIPTION`  | `<Framework name> automated run`        | No       | Any string                 |
| No        | Qase test run complete                                                                                                | `testops.run.complete`     | `QASE_TESTOPS_RUN_COMPLETE`     | `True`                                  |          | `True`, `False`            |
| No        | Qase test plan ID                                                                                                     | `testops.plan.id`          | `QASE_TESTOPS_PLAN_ID`          |  undefined                              | No       | Any integer                |
| No        | Size of batch for sending test results                                                                                | `testops.batch.size`       | `QASE_TESTOPS_BATCH_SIZE`       | `200`                                   | No       | Any integer                |
| No        | Enable defects for failed test cases                                                                                  | `testops.defect`           | `QASE_TESTOPS_DEFECT`           | `False`                                 | No       | `True`, `False`            |
### Example `qase.config.json` config:

```json
{
  "mode": "testops",
  "fallback": "report",
  "debug": false,
  "environment": "local",
  "captureLogs": false,
  "report": {
    "driver": "local",
    "connection": {
      "local": {
        "path": "./build/qase-report",
        "format": "json"
      }
    }
  },
  "testops": {
    "api": {
      "token": "<token>",
      "host": "qase.io"
    },
    "run": {
      "title": "Regress run",
      "description": "Regress run description",
      "complete": true
    },
    "defect": false,
    "project": "<project_code>",
    "batch": {
      "size": 100
    }
  }
}
```
