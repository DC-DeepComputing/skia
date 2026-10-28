---
name: skia/assert-subrange-within-parent-bounds
description: When extracting or computing subranges, slices, or sub-allocations from a parent range, assert with SkASSERT that subrange bounds remain within the parent
rubric: correctness
severity: should_fix
---

**Rule.** When extracting, slicing, or calculating a subrange, subview, or
sub-allocation from a parent range or container, assert with SkASSERT that the
slice offset is non-negative and the requested subrange does not exceed the
parent's bounds (e.g., SkASSERT((sub.offset + sub.count) <= (parent.offset +
parent.count))). Never assume a subrange calculation automatically stays within
parent limits without explicit invariant validation.

**Why.** A slice or subview of an existing range should only winnow down the
range and never break out of the parent's bounds. Asserting range containment
catches offset arithmetic bugs, out-of-bounds calculations, and parent-child
desynchronization early during debug builds and test execution.

## Examples

Bad:

```cpp
SlotRange fixedSlotRange(Generator* gen) override {
    SlotRange range = fParent->fixedSlotRange(gen);
    range.index += fInitialSlot;
    range.count = fNumSlots;
    return range;
}
```

Good:

```cpp
SlotRange fixedSlotRange(Generator* gen) override {
    SlotRange range = fParent->fixedSlotRange(gen);
    SlotRange adjusted = range;
    adjusted.index += fInitialSlot;
    adjusted.count = fNumSlots;
    SkASSERT((adjusted.index + adjusted.count) <= (range.index + range.count));
    return adjusted;
}
```
