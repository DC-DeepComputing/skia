---
name: skia/assert-bounds-before-end-relative-offset-calculations
description: When calculating end-relative offsets or reverse indices by subtracting from total counts, assert with SkASSERT that the index is within bounds
rubric: correctness
severity: should_fix
---

**Rule.** When computing end-relative offsets, stack depths, or reverse indices
by subtracting from a total count (e.g., total - index - 1), assert with
SkASSERT that the index is within valid bounds (e.g., SkASSERT(index < total) or
(unsigned)index < (unsigned)total) before performing the calculation. Do not
pass unasserted signed difference offsets to downstream buffer or stack
operations where out-of-range values produce negative offsets.

**Why.** In Skia, counts, indices, and offsets are represented using signed
integers. In reverse index calculations such as total - index - 1, an
out-of-bounds index produces a negative offset that can cause stack underflow,
invalid container indexing, or memory corruption. Asserting index preconditions
ensures bounds violations are caught early in debug builds.

## Examples

Bad:

```cpp
[[nodiscard]] bool push(Generator* gen, Slot slot) override {
    // Unchecked subtraction produces negative depth if slot >= numSlots().
    fDedicatedStack->pushClone(1, this->numSlots() - slot - 1);
    return true;
}
```

Good:

```cpp
[[nodiscard]] bool push(Generator* gen, Slot slot) override {
    SkASSERT(slot < this->numSlots());
    fDedicatedStack->pushClone(1, this->numSlots() - slot - 1);
    return true;
}
```
