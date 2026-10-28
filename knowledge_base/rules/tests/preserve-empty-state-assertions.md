---
name: skia/tests/preserve-empty-state-assertions
description: When updating unit tests to reflect changed semantics for non-empty edge cases, preserve explicit test assertions for genuinely empty inputs
rubric: testing
severity: should_fix
---

**Rule.** When updating unit tests to reflect changed semantics for non-empty or
non-trimmed edge cases, preserve explicit test coverage for genuinely empty
inputs. Retain or add explicit assertions verifying that empty containers or
zero-length inputs still yield empty objects or expected empty-state values,
rather than replacing or inverting existing empty-state assertions.

**Why.** Modifying or inverting edge-case test assertions to accommodate changed
trimming, padding, or degenerate-input semantics can inadvertently delete
coverage for true zero-length inputs. Retaining explicit assertions for
genuinely empty states prevents regressions where basic zero-element creation or
initialization paths break unnoticed.

## Examples

Bad:

```cpp
const SkPathVerb move[] = { M };  // the M will not be trimmed
REPORTER_ASSERT(reporter, !SkPathData::Make({pts, 1}, move, {})->empty());
```

Good:

```cpp
const SkPathVerb move[] = { M };  // the M will not be trimmed
REPORTER_ASSERT(reporter, SkPathData::Make({}, {}, {})->empty());
REPORTER_ASSERT(reporter, !SkPathData::Make({pts, 1}, move, {})->empty());
```
