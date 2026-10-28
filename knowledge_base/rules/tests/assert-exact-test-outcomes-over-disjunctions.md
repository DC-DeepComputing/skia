---
name: skia/tests/assert-exact-test-outcomes-over-disjunctions
description: When verifying deterministic operations in unit tests with REPORTER_ASSERT, assert the specific expected outcome or status code rather than loose inequalities or permissive disjunctions
rubric: testing
severity: should_fix
---

**Rule.** In unit tests, do not use disjunctive (`||`) conditions or loose
inequality checks (such as `!= kSuccess`) in `REPORTER_ASSERT` when validating
deterministic operations, decodes, or failure modes. Assert the specific
expected outcome, error code, or status (such as `result ==
SkCodec::kIncompleteInput`) directly.

**Why.** Unit tests execute under controlled, deterministic conditions where
expected results and failure modes are precisely known. Permissive disjunctions
and loose inequality checks weaken test verification and can allow regressions,
unexpected errors, or unhandled failure states to pass unnoticed.

## Examples

Bad:

```cpp
// Loose inequality and disjunction allow unexpected errors or unintended states to pass:
REPORTER_ASSERT(r, decodeResult != SkCodec::kSuccess || info.height() == 0,
                "Should not successfully decode with incomplete pixel data, got %s",
                SkCodec::ResultToString(decodeResult));
```

Good:

```cpp
// Assert the exact expected status code for the deterministic partial input:
REPORTER_ASSERT(r, decodeResult == SkCodec::kIncompleteInput,
                "Expected kIncompleteInput with partial pixel data, got %s",
                SkCodec::ResultToString(decodeResult));
```
