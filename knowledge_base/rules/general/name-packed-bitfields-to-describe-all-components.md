---
name: skia/name-packed-bitfields-to-describe-all-components
description: When naming packed integer fields or composite bitmasks, describe all packed components or use an encompassing descriptor name rather than omitting constituent flags
rubric: readability
severity: should_fix
---

**Rule.** Name packed integer fields, composite keys, and bitmasks to accurately
reflect all components they encode (e.g., naming both index and mask as in
fPackedLineIdxIntersectionMask, or using an encompassing suffix like Desc, Mask,
or Key) rather than naming only a subset of the packed fields. In structs and
classes with methods, also ensure member fields follow the 'f' prefix
convention.

**Why.** Naming a packed integer after only a subset of its contents (such as
naming a field containing both an index and an intersection mask as only an
index or winding) obscures the presence of other packed flags and misleads
readers about what data the field holds. Accurate composite names ensure
maintainers know all constituent components packed into the integer.

## Examples

Bad:

```cpp
struct Tile {
    // Packs lineIdx and intersectionMask (W | R | L | T | B)
    uint32_t packedWindingLineIdx;
};
```

Good:

```cpp
struct Tile {
    // Accurately describes both the line index and the full intersection mask
    uint32_t fPackedLineIdxIntersectionMask;
};
```
