---
name: skia/tests/prefer-direct-reporter-assert
description: In unit tests, assert conditions directly with REPORTER_ASSERT instead of wrapping logic in if-else branches; use ERRORF for explicit branch failure reporting
rubric: readability
severity: nit
---

**Rule.** In unit tests, assert boolean operations and preconditions directly
with `REPORTER_ASSERT(reporter, condition)` rather than wrapping test logic in
`if (condition) { ... } else { REPORTER_ASSERT(reporter, false, ...); }`. If an
explicit failure report is required in an unhandled branch or error path, use
`ERRORF(reporter, ...)` or `REPORT_FAILURE(...)` instead of passing `false` to
`REPORTER_ASSERT`.

**Why.** Direct assertions flatten test control flow, eliminate unnecessary
nesting, and automatically print the failed condition expression upon failure.
Calling `REPORTER_ASSERT(reporter, false)` obscures what condition was expected,
whereas `ERRORF` formats descriptive failure messages when control flow reaches
an unexpected state.

## Examples

Bad:

```cpp
if (surface->peekPixels(&pixmap)) {
    // test body
} else {
    REPORTER_ASSERT(reporter, false, "Could not read pixels");
}
```

Good:

```cpp
REPORTER_ASSERT(reporter, surface->peekPixels(&pixmap));
// test body

// When explicit branch failure reporting is necessary:
if (unhandledType) {
    ERRORF(reporter, "Unhandled surface type: %d", type);
    return;
}
```
