---
name: skia/prefer-skdata-make-zero-initialized
description: When allocating a zero-initialized SkData buffer, use static factory SkData::MakeZeroInitialized(size) instead of calling SkData::MakeUninitialized followed by sk_bzero or memset
rubric: readability
severity: nit
---

**Rule.** When creating an SkData instance whose buffer must be filled with
zeros, use the dedicated factory SkData::MakeZeroInitialized(size) rather than
calling SkData::MakeUninitialized(size) and manually clearing the memory with
sk_bzero or memset.

**Why.** SkData::MakeZeroInitialized allocates and zeroes the underlying memory
buffer in a single step, eliminating boilerplate manual clearing calls and
avoiding temporary uninitialized buffer states.

## Examples

Bad:

```cpp
auto uniformData = SkData::MakeUninitialized(fEffect->uniformSize());
SkASSERT(uniformData);
sk_bzero(uniformData->writable_data(), uniformData->size());
```

Good:

```cpp
auto uniformData = SkData::MakeZeroInitialized(fEffect->uniformSize());
SkASSERT(uniformData);
```
