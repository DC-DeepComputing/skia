---
name: skia/avoid-integer-accumulation-for-existence-checks
description: When testing for element presence or non-zero quantities across a collection, use a boolean accumulator or early exit instead of summing integer counts
rubric: correctness
severity: should_fix
---

**Rule.** Do not accumulate integer sums or counts across a collection merely to
check for element existence or non-zero status (e.g. sum > 0 or count != 0). Use
a boolean accumulator (such as hasMatch |= (count > 0)), an early-exit return
guard, or standard quantifier predicates like std::any_of instead.

**Why.** Summing values into an integer risks signed arithmetic overflow
(undefined behavior in C++) when iterating over large counts or unbounded
elements. In addition, accumulating an integer obscures the boolean intent of
the query and misses opportunities to short-circuit iteration upon encountering
the first matching element.

## Examples

Bad:

```cpp
bool ProxyReadCountMap::hasPendingReads() const {
    int readCount = 0;
    fCounts.foreach([&readCount](const TextureProxy*, int proxyReadCount) {
        readCount += proxyReadCount;
    });
    return readCount > 0;
}
```

Good:

```cpp
bool ProxyReadCountMap::hasPendingReads() const {
    bool hasPendingReads = false;
    fCounts.foreach([&hasPendingReads](const TextureProxy*, int proxyReadCount) {
        hasPendingReads |= (proxyReadCount > 0);
    });
    return hasPendingReads;
}
```
