---
name: skia/assert-bitfield-width-bounds-when-packing
description: When packing bitmasks or values into integers using bitwise shifts and OR, assert that inputs fit within allocated bit-widths using SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** When packing bitmasks, flags, or values into an integer with bitwise
shifts and OR operations, assert with SkASSERT that inputs do not exceed their
allocated bit-width (e.g., SkASSERT(value < (1 << kShift))) and contain no stray
bits outside the valid mask (SkASSERT((mask & ~kValidMask) == 0)). Verify input
bounds before or during packing to prevent bits from overflowing into adjacent
fields.

**Why.** When multiple fields share an integer, bits set beyond a field's
allocated width silently bleed into adjacent fields during bitwise OR, causing
subtle state corruption that is difficult to diagnose. Asserting bit-width
bounds in debug builds catches overflow at the packing call site immediately.

## Examples

Bad:

```cpp
Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
        : fX(x)
        , fY(y)
        , fPackedLineIdxIntersectionMask((lineIdx << IntersectionBits::INT_MASK_SHIFT) |
                                         intersectionMask) {}
```

Good:

```cpp
Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
        : fX(x)
        , fY(y)
        , fPackedLineIdxIntersectionMask((lineIdx << IntersectionBits::INT_MASK_SHIFT) |
                                         intersectionMask) {
    SkASSERT(intersectionMask < (1 << IntersectionBits::INT_MASK_SHIFT));
    SkASSERT((intersectionMask & ~IntersectionBits::kValidMask) == 0);
}
```
