---
name: skia/encapsulate-packed-bitfield-accessors
description: When a struct packs multiple logical fields into a composite integer member, provide member accessors to unpack them rather than bit-shifting at call sites
rubric: design
severity: nit
---

**Rule.** When a struct or class packs multiple logical fields or bitmasks into
an integer member variable, provide member accessor methods to extract each
component. Do not require callers or test code to open-code bitwise shifts and
masks on the packed member directly.

**Why.** Directly shifting and masking packed fields at call sites leaks the
internal bit layout, duplicates extraction logic across callers, and increases
maintenance costs when bit widths or offsets change. Member accessors
encapsulate unpacking in a single location and keep call sites readable.

## Examples

Bad:

```cpp
// Callers manually unpack packed bitfields:
uint32_t rawLine = tile.fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
uint32_t mask = tile.fPackedLineIdxIntersectionMask & IntersectionBits::INTERSECTION_MASK;
bool hasWinding = (tile.fPackedLineIdxIntersectionMask & IntersectionBits::W) != 0;
```

Good:

```cpp
struct Tile {
    uint32_t lineIdx() const {
        return fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
    }
    uint32_t intersectionMask() const {
        return fPackedLineIdxIntersectionMask & IntersectionBits::INTERSECTION_MASK;
    }
    bool hasWinding() const {
        return (fPackedLineIdxIntersectionMask & IntersectionBits::W) != 0;
    }
};

// Callers use encapsulated accessors:
uint32_t rawLine = tile.lineIdx();
uint32_t mask = tile.intersectionMask();
bool hasWinding = tile.hasWinding();
```
