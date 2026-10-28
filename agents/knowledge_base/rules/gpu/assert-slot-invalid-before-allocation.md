---
name: skia/gpu/assert-slot-invalid-before-allocation
description: When allocating or creating GPU resources in an indexed slot or page table, assert that the target slot is invalid or unallocated before assignment
rubric: readability
severity: should_fix
---

**Rule.** When defining allocation or creation methods that populate an indexed
slot or page table in GPU resource, cache, or atlas managers, assert upon entry
that the target slot is unallocated or invalid (e.g.,
SkASSERT(!fPages[slot].isValid())). Enforce this precondition before assigning
or initializing resources to maintain slot lifecycle invariants and prevent
accidental overwrites.

**Why.** Overwriting an indexed slot that already holds an active resource
without resetting or retiring it corrupts internal state and clobbers in-flight
GPU resources. Asserting slot invalidity on method entry catches lifecycle and
state-machine bugs at debug time before resources leak.

## Examples

Bad:

```cpp
bool AlphaAtlasManager::createPageInSlot(int32_t slot, int32_t minRequiredBytes) {
    SkASSERT(slot < SparseStripConfig::kMaxTexturePages);
    // Missing assertion that the target slot is unallocated
    fPages[slot] = this->allocatePage(minRequiredBytes);
    return fPages[slot].isValid();
}
```

Good:

```cpp
bool AlphaAtlasManager::createPageInSlot(int32_t slot, int32_t minRequiredBytes) {
    SkASSERT(slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());
    fPages[slot] = this->allocatePage(minRequiredBytes);
    return fPages[slot].isValid();
}
```
