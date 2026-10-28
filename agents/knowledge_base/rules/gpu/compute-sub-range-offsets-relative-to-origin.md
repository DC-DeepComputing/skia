---
name: skia/gpu/compute-sub-range-offsets-relative-to-origin
description: When packing or validating layout offsets for independent sub-ranges sharing a storage buffer, compute alignments and padding relative to the sub-range origin
rubric: correctness
severity: should_fix
---

**Rule.** When packing or validating element layouts for independent sub-ranges
(such as distinct GPU buffer bindings or substructs) sharing a single storage
buffer, compute element alignments, padding, and offsets relative to the
sub-range's origin (storage size minus starting offset) rather than absolute
buffer size. When validating destination pointers against tracked offsets,
subtract the sub-range's starting offset from the absolute pointer offset.

**Why.** A GPU buffer binding or substruct bound at an offset begins at local
byte 0. Aligning an absolute buffer size to an element's alignment can insert
spurious padding or misalign data if the absolute offset is not an exact
multiple of the alignment. Calculating offsets, padding, and pointer validations
relative to the sub-range start ensures GPU layout matches shader binding
expectations.

## Examples

Bad:

```cpp
// Flawed: aligning against absolute buffer size introduces spurious padding or misaligns fields
const int offset = fStorage.size();
const int padding = SkAlignTo(offset, alignment) - offset;
fOffsetCalculator = UniformOffsetCalculator::ForTopLevel(fLayout, offset);
...
int ptrOffset = static_cast<int>(reinterpret_cast<intptr_t>(dst) -
                                 reinterpret_cast<intptr_t>(fStorage.data()));
SkASSERT(ptrOffset == fOffsetCalculator.advanceOffset(type, count));
```

Good:

```cpp
// Correct: calculate offsets, padding, and pointer validation relative to sub-range origin
const size_t relativeOffset = fStorage.size() - fStartingOffset;
const size_t padding = SkAlignTo(relativeOffset, alignment) - relativeOffset;
fOffsetCalculator = UniformOffsetCalculator::ForTopLevel(fLayout, /*offset=*/0);
...
int ptrOffset = static_cast<int>(reinterpret_cast<intptr_t>(dst) -
                                 reinterpret_cast<intptr_t>(fStorage.data()) -
                                 fStartingOffset);
SkASSERT(ptrOffset == fOffsetCalculator.advanceOffset(type, count));
```
