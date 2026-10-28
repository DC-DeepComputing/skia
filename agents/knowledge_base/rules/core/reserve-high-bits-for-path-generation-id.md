---
name: skia/core/reserve-high-bits-for-path-generation-id
description: When allocating path generation IDs that reserve space for fill type or metadata, clear the high bits rather than low bits
rubric: correctness
severity: should_fix
---

**Rule.** When generating or allocating unique path generation IDs that leave
space for SkPathFillType or client metadata, reserve and clear the high bits
(bits 30..31) rather than shifting or masking the low bits. Maintain zero in the
high bits so external callers can pack metadata into the upper bits without
altering the generation identifier.

**Why.** Downstream clients, such as Android Framework, pack SkPathFillType into
the top two bits of path generation IDs for caching and draw state invalidation.
Reserving the low bits instead leaves non-zero values in the upper bits, causing
generation ID collisions and corrupting cached client state.

## Examples

Bad:

```cpp
constexpr int kShiftToMakeRoomForFillType = 2;
...
id = nextID.fetch_add(1, std::memory_order_relaxed);
// Anti-pattern: shifting left reserves low bits, leaving high bits occupied
id <<= kShiftToMakeRoomForFillType;
```

Good:

```cpp
constexpr int kHighBitsToMakeRoomForFillType = 2;
...
id = nextID.fetch_add(1, std::memory_order_relaxed);
// Clear the high bits to leave room for client metadata (such as fill type)
id <<= kHighBitsToMakeRoomForFillType;
id >>= kHighBitsToMakeRoomForFillType;
```
