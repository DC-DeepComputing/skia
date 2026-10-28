---
name: skia/do-not-assert-arbitrary-limits-on-valid-inputs
description: When asserting input ranges or bounds with SkASSERT, do not enforce arbitrary limits or fuzzer-specific constraints on functionally valid inputs
rubric: correctness
severity: should_fix
---

**Rule.** Do not use SkASSERT to enforce arbitrary input ranges, heuristic
thresholds, or limits introduced solely to avoid fuzzer timeouts or enforce
caller assumptions when the algorithm is functionally sound outside that range.
Verify only true structural invariants required for correctness and memory
safety in library code, and constrain synthetic inputs or high-complexity test
cases within fuzzer harnesses instead.

**Why.** Assertions (SkASSERT) must verify inviolable program invariants.
Enforcing artificial bounds or fuzzer caps in library code causes debug builds
and test suites to crash on valid inputs that the algorithm could otherwise
handle safely, conflating fuzzer resource limits with internal corruption.

## Examples

Bad:

```cpp
// Bad: restricting input range in library code to prevent excessive work or fuzzer timeouts
// when the algorithm is mathematically and functionally valid outside this range.
SkASSERT(offset >= -1000 && offset <= 1000);
```

Good:

```cpp
// Good: verify true invariants (e.g. finite values) in library code, and bound synthetic inputs in fuzzer harnesses.
SkASSERT(SkIsFinite(offset));

// In fuzz harness (e.g., fuzz/FuzzPolyUtils.cpp):
// SkScalar offset = fuzz->nextRange(-1000.0f, 1000.0f);
```
