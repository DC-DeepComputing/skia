---
name: skia/qualify-constants-instead-of-inheriting
description: When using constants, bitmasks, or static helpers from utility structs, qualify them explicitly with the scope name instead of inheriting from the struct
rubric: design
severity: should_fix
---

**Rule.** Do not inherit from a struct or class solely to import its constants,
bitmasks, enum values, or static helper members into local scope. Access static
members and constants explicitly via scope qualification (e.g.,
Helper::kConstant or MaskDefs::kShift).

**Why.** Inheritance models an 'is-a' subtyping relationship and introduces
unnecessary coupling into the type hierarchy when used merely to avoid typing
scope qualifiers. Explicit scope qualification clarifies the source of constants
and keeps data structs decoupled from unrelated utility types.

## Examples

Bad:

```cpp
// Bad: inheriting from a utility struct solely to import bitmask constants.
struct Tile : public IntersectionBits {
    Tile(uint32_t lineIdx, uint32_t mask)
        : fPackedLineIdxIntersectionMask((lineIdx << INT_MASK_SHIFT) | mask) {}
    uint32_t fPackedLineIdxIntersectionMask;
};
```

Good:

```cpp
// Good: reference constants with explicit scope qualification.
struct Tile {
    Tile(uint32_t lineIdx, uint32_t mask)
        : fPackedLineIdxIntersectionMask(
                  (lineIdx << IntersectionBits::INT_MASK_SHIFT) | mask) {}
    uint32_t fPackedLineIdxIntersectionMask;
};
```
