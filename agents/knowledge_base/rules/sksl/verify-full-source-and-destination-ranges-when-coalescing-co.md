---
name: skia/sksl/verify-full-source-and-destination-ranges-when-coalescing-co
description: When coalescing adjacent copy operations into wider block copies in SkSL code generation, verify that full expanded source and destination ranges do not overlap
rubric: correctness
severity: should_fix
---

**Rule.** When coalescing adjacent copy or assignment instructions into a single
wider block copy, verify that the full expanded source range does not overlap
with the full expanded destination range. Do not check only the newly appended
range against the previous or expanded ranges, as earlier source slots may
overlap later destination slots.

**Why.** Sequential copies with overlapping source and destination ranges depend
on sequential execution order. Coalescing overlapping copies into a simultaneous
block copy alters program behavior and triggers undefined behavior when executed
via memcpy.

## Examples

Bad:

```cpp
// Flawed: only checks the appended chunk against the expanded destination
if (!slot_ranges_overlap(src, {lastInstr.fSlotA, lastInstr.fImmA + dst.count})) {
    lastInstr.fImmA += dst.count;
    return;
}
```

Good:

```cpp
// Correct: checks the full expanded source range against the full expanded destination range
if (!slot_ranges_overlap({lastInstr.fSlotB, lastInstr.fImmA + dst.count},
                         {lastInstr.fSlotA, lastInstr.fImmA + dst.count})) {
    lastInstr.fImmA += dst.count;
    return;
}
```
