---
name: skia/validate-inputs-against-boundary-sentinels
description: When using boundary sentinel elements to terminate search or traversal loops, ensure extreme inputs cannot satisfy the loop condition, or enforce explicit iterator bounds checks
rubric: correctness
severity: should_fix
---

**Rule.** When relying on sentinel elements at container or sequence boundaries
to terminate search or traversal loops without container end checks, ensure that
extreme input values (such as minimum or maximum integer limits) cannot satisfy
the loop predicate. Either validate, clamp, or pre-filter inputs strictly within
sentinel limits, or enforce explicit container bound checks (e.g., cursor !=
container.end()) in the loop condition.

**Why.** Boundary sentinels are commonly used to eliminate repeated
container-end checks in inner loops. However, if an input value matches or
exceeds the sentinel's extreme boundary values, the loop condition remains
satisfied when reaching the sentinel, causing traversal to advance past
container boundaries and trigger undefined behavior or out-of-bounds memory
access.

## Examples

Bad:

```cpp
// Left and right sentinels defined with INT32_MIN and INT32_MAX coordinates
for (auto cursor = insertionPoint; check(*cursor); ++cursor) {
    // If segment coordinates reach INT32_MAX, check(*cursor) matches the sentinel,
    // advancing cursor beyond fStatus.end() into unallocated memory.
    fCrossings.recordCrossing(segment, *cursor);
}
```

Good:

```cpp
// Pre-filter/validate inputs strictly within sentinel limits upfront,
// or include an explicit container bound check in the loop condition:
for (auto cursor = insertionPoint; cursor != fStatus.end() && check(*cursor); ++cursor) {
    fCrossings.recordCrossing(segment, *cursor);
}
```
