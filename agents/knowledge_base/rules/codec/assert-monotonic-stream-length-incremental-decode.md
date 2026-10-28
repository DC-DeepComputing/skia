---
name: skia/codec/assert-monotonic-stream-length-incremental-decode
description: When tracking stream progress or caching stream length in incremental decoding to skip redundant work, assert with SkASSERT that stream length increases monotonically
rubric: readability
severity: nit
---

**Rule.** When caching or tracking stream lengths in incremental decoding or
streaming parsers to avoid redundant work, assert that the stream length
increases monotonically using SkASSERT (e.g., `SkASSERT(currentLength >=
oldLength)` or `SkASSERT(currentLength > oldLength)` after an equality bailout).
Do not update cached length tracking or return early without verifying that the
stream has not been truncated or rewound.

**Why.** Incremental decoding assumes that incoming stream data only grows over
time as additional chunks arrive. Asserting monotonicity documents this
invariant and immediately catches unexpected stream rewinding, truncation, or
invalid stream re-entry during debug builds and fuzzing.

## Examples

Bad:

```cpp
if (oldLength == currentLength) {
    break;
}
fCachedStreamLength = currentLength;
```

Good:

```cpp
if (oldLength == currentLength) {
    break;
}
SkASSERT(currentLength > oldLength);
fCachedStreamLength = currentLength;
```
