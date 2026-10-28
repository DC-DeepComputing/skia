---
name: skia/avoid-zero-sentinel-for-valid-offsets
description: When tracking unset or inactive offsets and indices where zero is a valid position, use -1 or std::optional instead of zero as the sentinel
rubric: correctness
severity: nit
---

**Rule.** Do not use 0 as a sentinel value to indicate an unset, inactive, or
uninitialized offset or index when 0 is within the domain of valid positions.
Use an explicit negative sentinel value (such as -1) or std::optional<T>
instead.

**Why.** A buffer, struct, array, or layout element legitimately begins at
offset or index 0. Using 0 as a sentinel value creates ambiguity between an
inactive state and a valid entry located at the initial position, causing
assertions and state guards to misinterpret active elements as inactive.

## Examples

Bad:

```cpp
// 0 is ambiguous if an element or substruct legitimately starts at offset 0
int fSubstructStartingOffset = 0;
...
// Incorrectly conflates inactive state with a valid active element at offset 0
SkASSERT(fSubstructStartingOffset == 0);
```

Good:

```cpp
// -1 or std::optional unambiguously represents unset when 0 is a valid position
int fSubstructStartingOffset = -1;
...
// Unambiguously checks that no substruct is active
SkASSERT(fSubstructStartingOffset == -1);
```
