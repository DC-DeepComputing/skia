---
name: skia/api/correlate-bitfield-diagram-symbols
description: When documenting packed structs or bitfields that include an ASCII bit layout diagram, annotate prose field descriptions with corresponding diagram symbols
rubric: readability
severity: nit
---

**Rule.** When documenting packed structs, bitfields, or bitmask words that
include an ASCII bit layout diagram, annotate high-level list items and prose
field descriptions with the exact abbreviations or symbols used in the diagram
(e.g., `coarse winding (W)` or `intersection mask (R | L | T | B)`). Ensure
every letter code or multi-bit partition depicted in the diagram is explicitly
defined in the surrounding comments.

**Why.** ASCII bit diagrams frequently use compact single-letter symbols or
abbreviations to fit within column limits. Annotating high-level prose
descriptions with those exact symbols allows readers to immediately map
conceptual struct members to specific bit allocations without manually deducing
meanings from bit positions or implementation code.

## Examples

Bad:

```cpp
// A packed tile key, contains:
//  1) The tile coordinates in device space.
//  2) The coarse winding of the tile.
//  3) The intersection edge mask of the tile.
//  4) The index of the parent polyline.
struct Tile {
    ...
    /// MSB                                                    LSB
    /// 31----------------------------------------------5|4|3|2|1|0|
    /// |            Parent Line Index (27 bits)         |W|R|L|B|T|
    uint32_t fPackedLineIdxIntersectionMask;
};
```

Good:

```cpp
// A packed tile key, contains:
//  1) The tile coordinates in device space.
//  2) The coarse winding (W) of the tile.
//  3) The intersection edge mask (R | L | B | T) of the tile.
//  4) The index of the parent polyline.
struct Tile {
    ...
    /// MSB                                                    LSB
    /// 31----------------------------------------------5|4|3|2|1|0|
    /// |            Parent Line Index (27 bits)         |W|R|L|B|T|
    uint32_t fPackedLineIdxIntersectionMask;
};
```
