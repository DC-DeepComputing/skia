---
name: skia/test-emptiness-not-expected-size
description: When branching on optional or variable-length data presence, test explicitly for emptiness rather than expected non-zero sizes, and assert expected sizes with SkASSERT
rubric: readability
severity: should_fix
---

**Rule.** When branching on whether optional or variable-length data is present,
test explicitly for emptiness (e.g. `count == 0`, `count > 0`, or `.empty()`)
rather than checking against the expected non-zero payload size. In the
non-empty branch, verify expected size invariants using `SkASSERT` (or
`SkASSERTF`).

**Why.** Checking against an expected non-zero size to guard data processing
conflates emptiness checking with format validation. If corrupted or unexpected
non-zero data is received, equality checks silently bypass processing and fall
through to default paths rather than failing fast on invariant violations in
debug builds.

## Examples

Bad:

```cpp
// Obscures empty-check intent and silently falls through on invalid non-zero counts
if (dataIndexCount == 3) {
    SamplerDesc s(keyData[currentIndex], keyData[currentIndex+1], keyData[currentIndex+2]);
    append_sampler_desc(str, s);
}
```

Good:

```cpp
if (dataIndexCount == 0) {
    str->append("(0)");
} else {
    SkASSERT(dataIndexCount == 3);
    SamplerDesc s(keyData[currentIndex], keyData[currentIndex+1], keyData[currentIndex+2]);
    append_sampler_desc(str, s);
}
```
