---
name: skia/tests/disable-test-debug-abort-macros
description: When authoring or updating unit tests, re-comment or remove temporary local debugging abort macros and early exits before submitting code for review
rubric: readability
severity: nit
---

**Rule.** When authoring or modifying unit tests, re-comment or remove temporary
local debugging hooks, early exits, and abort macros (such as #define ...
SK_ABORT()) before submitting code. Ensure test failures are recorded
programmatically through REPORTER_ASSERT or ERRORF rather than aborting test
execution prematurely.

**Why.** Active debug abort hooks and premature exits cause the test runner (dm)
or CI test bots to crash on the first failure instead of recording assertion
errors and completing subsequent test cases. Leaving local debugging overrides
enabled masks test results across the test suite.

## Examples

Bad:

```cpp
// Uncomment the SK_ABORT() to exit on first pixel mismatch to help debugging
#define STOP_ON_TRANSFER_FAILURE SK_ABORT();
```

Good:

```cpp
// Uncomment the SK_ABORT() to exit on first pixel mismatch to help debugging
#define STOP_ON_TRANSFER_FAILURE // SK_ABORT();
```
