---
name: skia/prefer-skdebugfail-over-assert-false
description: When signaling debug failures on unexpected or unimplemented branches with fallback returns, use SkDEBUGFAIL or SkDEBUGFAILF instead of bare SkASSERT(false)
rubric: readability
severity: nit
---

**Rule.** When triggering debug-only assertion failures on unexpected or
unimplemented branches that retain fallback logic for release builds, use
SkDEBUGFAIL("message") or SkDEBUGFAILF(...) rather than bare SkASSERT(false) or
SkASSERT(0). Provide an explanatory message indicating why the branch was
reached or what functionality is unimplemented.

**Why.** SkASSERT(false) and SkASSERT(0) abort without explanatory context in
debug builds, complicating diagnosis when unexpected paths are hit. Using
SkDEBUGFAIL or SkDEBUGFAILF supplies actionable failure messages while cleanly
compiling out in release builds so non-fatal fallback behavior can proceed.

## Examples

Bad:

```cpp
case kBGR_101010x_XR_SkColorType:
    SkASSERT(false);
    return {UNORM, 10, 10, 10, 0, 20, 10, 0, 0};
```

Good:

```cpp
case kBGR_101010x_XR_SkColorType:
    SkDEBUGFAIL("Not implemented.");
    return {UNORM, 10, 10, 10, 0, 20, 10, 0, 0};
```
