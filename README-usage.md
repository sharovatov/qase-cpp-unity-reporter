## How to use

### TODO

- [x] support macro quick replacement for Unity's RUN_TEST
- [ ] publish it to PlatformIO for tests

### As of now

1. Download the reporter

```
git clone https://github.com/sharovatov/qase-cpp-unity-reporter.git
```

2. Copy the files into your project

- include/qase_reporter.h
- src/qase_reporter.cpp

3. Include the header in your project

```
#include "qase_reporter.h"
```

4. Update test entry points

You currently call Unity tests with:
```
UNITY_BEGIN();
RUN_TEST(your_test1);
RUN_TEST(your_test2);
UNITY_END();
```

These need to be replaced with:
```
// construct http client
// and config
QASE_UNITY_BEGIN();
QASE_RUN_TEST(your_test1);
QASE_RUN_TEST(your_test2);
QASE_UNITY_END(http, cfg);
```



  // test run with additional meta parameters
	QaseResultMeta meta = {
		.case_id = 42,
		.title = "WiFi connects",
		.fields = { {"severity", "critical"} }
	};
	QASE_RUN_TEST(test_wifi_connects_successfully, meta);
 
