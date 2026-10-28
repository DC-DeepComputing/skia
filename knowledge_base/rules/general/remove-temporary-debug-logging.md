---
name: skia/remove-temporary-debug-logging
description: When modifying production code or unit tests, remove temporary debug logging (printf, SkDebugf) and assert test expectations programmatically with REPORTER_ASSERT
rubric: change_scope
severity: should_fix
---

**Rule.** Remove temporary debug logging and ad-hoc print statements—such as
SkDebugf, SkDEBUGF, SKIA_LOG_D, SKGPU_LOG_D, printf, or std::cout—from
production source files and headers before submitting code for review. In unit
tests, do not leave temporary print statements to inspect values; assert test
expectations programmatically using REPORTER_ASSERT or report failures via
ERRORF. If permanent diagnostic logging is genuinely required in production
code, use the appropriate subsystem logging macro or guard it behind a dedicated
debug configuration flag.

**Why.** Temporary debug prints added during development or troubleshooting
pollute console, test runner (dm), and system logcat output, clutter code review
diffs, and can degrade runtime performance. In unit tests, debug prints fail to
verify behavior automatically, whereas REPORTER_ASSERT and ERRORF integrate
directly with test failure reporting.

## Examples

Bad:

```cpp
// Production code:
outEffectiveElements->push_back(&e);
SkDebugf("MSAA-based clip required\n");
break;

// Unit test:
printf("firstPoint at (%f, %f): %08X\n", firstPoint.x(), firstPoint.y(), firstPointColor);
```

Good:

```cpp
// Production code:
outEffectiveElements->push_back(&e);
break;

// Unit test:
REPORTER_ASSERT(reporter, actual == expected,
                "color mismatch at (%f, %f)", pt.x(), pt.y());
```
