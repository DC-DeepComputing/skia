---
name: skia/loop-atomic-counter-past-invalid-unique-id
description: When generating unique IDs with an atomic counter, loop fetch_add until the ID differs from SK_InvalidUniqueID to handle wraparound
rubric: correctness
severity: should_fix
---

**Rule.** When generating unique IDs using an atomic counter where
SK_InvalidUniqueID (or 0) serves as an invalid sentinel, loop fetch_add in a
do-while loop until the returned ID is not SK_InvalidUniqueID. Initialize the
counter past the sentinel (e.g., SK_InvalidUniqueID + 1 or 1) so neither initial
allocations nor 32-bit counter wraparounds return an invalid sentinel ID.

**Why.** SK_InvalidUniqueID is used throughout Skia to signify uninitialized,
unallocated, or invalid resources. If an unchecked 32-bit counter wraps around,
it returns the sentinel ID, causing valid objects to be treated as invalid and
corrupting cache lookups or lifetime checks.

## Examples

Bad:

```cpp
uint32_t next_unique_id() {
    static std::atomic<uint32_t> nextID{1};
    // Wraparound can yield SK_InvalidUniqueID (0).
    return nextID.fetch_add(1, std::memory_order_relaxed);
}
```

Good:

```cpp
uint32_t next_unique_id() {
    static std::atomic<uint32_t> nextID{SK_InvalidUniqueID + 1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}
```
