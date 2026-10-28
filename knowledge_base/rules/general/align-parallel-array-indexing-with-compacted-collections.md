---
name: skia/align-parallel-array-indexing-with-compacted-collections
description: When populating parallel or auxiliary output arrays while filtering or deduplicating collections, index by the compacted count or append in lockstep rather than using the raw loop index
rubric: correctness
severity: should_fix
---

**Rule.** Do not index auxiliary or parallel output arrays using the raw input
loop index when filtering, compacting, or deduplicating elements into a separate
collection. Index parallel output arrays using the compacted collection's
current size (or append to both collections in lockstep) so element indices
remain synchronized.

**Why.** Indexing parallel output arrays with the raw input loop index skips
slots whenever duplicate or filtered elements are omitted or merged. This leaves
gaps of uninitialized elements in the output arrays and misaligns downstream
lookups that expect 1:1 index parity with the compacted collection.

## Examples

Bad:

```cpp
for (int i = 0; i < info.fColorCount; i++) {
    if (i > 0 && info.fColorOffsets[i-1] == info.fColorOffsets[i]) {
        afterColors[colorOffsets.size() - 1] = info.fColors[i];
    } else {
        colorOffsets.push_back(info.fColorOffsets[i]);
        beforeColors[i] = info.fColors[i]; // Bug: indexed by raw input loop index 'i'
    }
}
```

Good:

```cpp
for (int i = 0; i < info.fColorCount; i++) {
    if (i > 0 && info.fColorOffsets[i-1] == info.fColorOffsets[i]) {
        afterColors[colorOffsets.size() - 1] = info.fColors[i];
    } else {
        colorOffsets.push_back(info.fColorOffsets[i]);
        size_t idx = colorOffsets.size() - 1;
        beforeColors[idx] = info.fColors[i]; // Aligned with compacted colorOffsets
    }
}
```
