---
name: skia/name-spilled-data-as-overflow-not-null
description: When naming storage, entries, or helpers for data that exceeded buffer or atlas capacity, use overflow or spill rather than null
rubric: readability
severity: should_fix
---

**Rule.** Do not use 'null' (such as NullCaps or fNullBuffer) to name valid
data, entries, or storage buffers that exceeded primary capacity and spilled
over. Name them using explicit 'overflow' or 'spill' terminology (such as
resolveOverflowCaps or fOverflowBuffer) to reflect their capacity-exceeded state
accurately.

**Why.** The term 'null' implies that an object is absent, invalid, or empty
(such as a null pointer or empty geometry). Valid entries that merely exceeded
texture, atlas, or buffer capacity and were diverted to secondary storage remain
valid data; labeling them 'null' obscures their validity and misleads callers
into treating them as missing or uninitialized.

## Examples

Bad:

```cpp
// Misleading: 'null' implies geometry or entries are invalid or missing.
bool resolveNullCaps(EndCaps* ends);
skia_private::TArray<uint8_t> fNullBuffer;
bool hasNullCaps() const { return fFirstNullCapIndex != kInvalidIndex; }
```

Good:

```cpp
// Accurate: 'overflow' indicates valid entries that exceeded atlas capacity.
bool resolveOverflowCaps(EndCaps* ends);
skia_private::TArray<uint8_t> fOverflowBuffer;
bool hasOverflowCaps() const { return fFirstOverflowCapIndex != kInvalidIndex; }
```
