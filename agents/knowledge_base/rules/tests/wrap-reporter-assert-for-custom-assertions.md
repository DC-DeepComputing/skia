---
name: skia/tests/wrap-reporter-assert-for-custom-assertions
description: When defining custom assertion helpers in unit tests, wrap REPORTER_ASSERT in a macro rather than directly calling reportFailedWithContext or plumbing __FILE__ and __LINE__
rubric: readability
severity: should_fix
---

**Rule.** In unit tests, use REPORTER_ASSERT (or define a macro wrapper around
REPORTER_ASSERT for custom reusable assertions) rather than directly calling
Reporter::reportFailedWithContext or manually passing __FILE__ and __LINE__.
Keep test assertions standard and let REPORTER_ASSERT capture call-site source
locations and condition expressions.

**Why.** REPORTER_ASSERT already encapsulates failure reporting, condition
stringification, and __FILE__/__LINE__ tracking. Directly calling
reportFailedWithContext bypasses standard test assertion conventions and
requires tedious manual plumbing of source locations, whereas a macro wrapping
REPORTER_ASSERT preserves accurate call-site reporting cleanly.

## Examples

Bad:

```cpp
bool AssertCodecResult(skiatest::Reporter* r,
                       SkCodec::Result actualResult,
                       SkCodec::Result expectedResult,
                       const char* condition,
                       const char* file = __FILE__,
                       int line = __LINE__) {
    if (actualResult != expectedResult) {
        SkString msg = SkStringPrintf("actualResult=%s != expectedResult=%s",
                                      SkCodec::ResultToString(actualResult),
                                      SkCodec::ResultToString(expectedResult));
        r->reportFailedWithContext(skiatest::Failure(file, line, condition, std::move(msg)));
        return false;
    }
    return true;
}
```

Good:

```cpp
#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))
```
