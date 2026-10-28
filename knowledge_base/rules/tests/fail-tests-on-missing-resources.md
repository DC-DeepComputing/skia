---
name: skia/tests/fail-tests-on-missing-resources
description: When handling missing test resources, explicitly fail required fixtures via ERRORF or REPORTER_ASSERT, but use INFOF when skipping optional resources
rubric: correctness
severity: should_fix
---

**Rule.** In unit tests, do not silently return or skip test execution when a
required test fixture, asset, or resource file (such as data loaded via
GetResourceAsData() or GetResourcePath()) fails to load; explicitly fail the
test via ERRORF(reporter, ...) or REPORTER_ASSERT. Conversely, when skipping a
test due to missing optional resources or unsupported environments (such as
non-checked-in fonts or optional codecs), do not use ERRORF (which marks the
test as failed); log an informational note with INFOF(reporter, ...) and return
early without failing.

**Why.** Silently returning on missing required assets masks missing files, path
typos, and checkout or sync failures, allowing tests to falsely pass without
exercising assertions. Conversely, logging missing optional resources with
ERRORF invokes REPORT_FAILURE and causes false-positive test failures on bots
lacking external or optional assets; INFOF records the diagnostic note while
allowing the test to skip gracefully.

## Examples

Bad:

```cpp
// Bad (required fixture): silently skipping hides missing required files
sk_sp<SkData> data = GetResourceAsData(file);
if (!data) {
    return;
}

// Bad (optional resource): ERRORF marks the test as failed on bots without optional fonts
if (!fontCollection->fontsFound()) {
    ERRORF(reporter, "No fonts found\n");
    return;
}
```

Good:

```cpp
// Good (required fixture): explicitly fail the test
sk_sp<SkData> data = GetResourceAsData(file);
if (!data) {
    ERRORF(reporter, "Missing resource: %s", file);
    return;
}

// Good (optional resource): INFOF logs diagnostically without failing the test
if (!fontCollection->fontsFound()) {
    INFOF(reporter, "No fonts found\n");
    return;
}
```
