---
name: skia/use-named-constants-for-array-indices
description: When accessing arrays or containers with established named index constants or enums, use the named constants instead of raw integer literals
rubric: readability
severity: nit
---

**Rule.** When named constants or enums are defined for container or array
indices, use them consistently across all accesses instead of hardcoding raw
integer literals. Do not mix hardcoded numeric literals with named index
constants when indexing or checking container elements.

**Why.** Using raw integer literals for indices when named constants or enums
exist reduces code readability, obscures semantic intent, defeats searchability,
and introduces silent out-of-sync bugs if array layouts or indices are ever
reordered or refactored.

## Examples

Bad:

```cpp
static constexpr int kOuter = 0;
static constexpr int kInner = 1;
// ...
// Bad: hardcoding literal 1 when kInner is defined for this slot
bool usesSource = inputs[1] ? as_IFB(inputs[kInner])->usesSource() : false;
```

Good:

```cpp
static constexpr int kOuter = 0;
static constexpr int kInner = 1;
// ...
// Good: consistently use named constants for all indexed accesses
bool usesSource = inputs[kInner] ? as_IFB(inputs[kInner])->usesSource() : false;
```
