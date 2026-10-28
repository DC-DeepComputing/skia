---
name: skia/modules/place-jsonreader-tests-in-tests-jsontest
description: When adding unit tests for modules without standalone test suites like modules/jsonreader, add cases to existing tests/ suites rather than creating new targets
rubric: design
severity: should_fix
---

**Rule.** When adding unit tests for modules/jsonreader, add test cases directly
to the established tests/JSONTest.cpp suite rather than creating a new test file
or test directory under modules/jsonreader/tests/. Do not introduce new test
build targets in GN or Bazel for modules whose tests are already centralized in
tests/.

**Why.** modules/jsonreader does not maintain its own test runner or test
subdirectory; all skjson and SkJSONReader unit tests are co-located in
tests/JSONTest.cpp and run via dm. Introducing ad-hoc module test files and
targets creates unnecessary build system churn and fragments parser test
coverage.

## Examples

Bad:

```cpp
// modules/jsonreader/tests/SkJSONReaderTest.cpp
#include "modules/jsonreader/SkJSONReader.h"
#include "tests/Test.h"

DEF_TEST(SkjsonFloatParsing_ScientificNotation, reporter) {
    skjson::DOM dom("{\"num\": 1e2}", 14);
    REPORTER_ASSERT(reporter, dom.root().is<skjson::ObjectValue>());
}
```

Good:

```cpp
// tests/JSONTest.cpp
#include "modules/jsonreader/SkJSONReader.h"
#include "tests/Test.h"

DEF_TEST(JSON_ParseScientificNotation, reporter) {
    skjson::DOM dom("{\"num\": 1e2}", 14);
    REPORTER_ASSERT(reporter, dom.root().is<skjson::ObjectValue>());
}
```
