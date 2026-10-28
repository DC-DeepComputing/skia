---
name: skia/use-size-not-size-bytes-for-container-capacity
description: When tracking container capacity, high-water marks, or reservations in Skia containers, query size() rather than size_bytes() to avoid unit confusion and casts
rubric: readability
severity: should_fix
---

**Rule.** When tracking container sizes, peak utilization, or high-water marks
for capacity planning and reservations (such as reserve() or reserve_exact()),
query size() rather than size_bytes(). Skia container capacity and reservation
methods operate on element counts (T) rather than raw bytes; using size() avoids
unit confusion, eliminates redundant static_cast<int> conversions from size_t,
and prevents sizing bugs if element types change.

**Why.** Container allocation methods (such as reserve_exact(), reserve(), and
capacity()) measure capacity in elements of type T, and TArray::size() already
returns int. Using size_bytes() introduces unit confusion between byte counts
and element counts, forces boilerplate static_cast<int> conversions because
size_bytes() returns size_t, and creates silent sizing bugs if container element
types are refactored from single-byte to multi-byte types.

## Examples

Bad:

```cpp
// Bad: size_bytes() returns size_t bytes, forcing an explicit cast and risking unit confusion
fStorageHighWaterMark = std::max(fStorageHighWaterMark,
                                 static_cast<int>(fStorage.size_bytes()));
...
if (fStorageHighWaterMark < halfCapacity) {
    fStorage.reserve_exact(halfCapacity);
}
```

Good:

```cpp
// Good: size() returns element count as int, matching capacity and reservation methods
fStorageHighWaterMark = std::max(fStorageHighWaterMark, fStorage.size());
...
if (fStorageHighWaterMark < halfCapacity) {
    fStorage.reserve_exact(halfCapacity);
}
```
