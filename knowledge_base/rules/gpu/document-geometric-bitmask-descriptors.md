---
name: skia/gpu/document-geometric-bitmask-descriptors
description: When defining bitmasks or packed flag descriptors for geometric or rasterization state, document boundary tie-breaking rules and non-intuitive flag semantics
rubric: readability
severity: nit
---

**Rule.** When defining bitmasks, bitfields, or packed flag descriptors that
represent geometric or rasterization state (such as tile edge intersections,
boundary touches, outcodes, or coarse winding), document their exact semantics
and invariants in an accompanying comment. Explicitly clarify where flag
definitions deviate from intuitive geometric properties due to corner
tie-breaking rules, differing inclusive/exclusive boundary conventions, or
winding accumulation.

**Why.** Bit flags in rasterization and geometry pipelines frequently encode
compact pipeline or hardware state rather than literal geometry. Without
explicit documentation of tie-breaking rules and boundary conventions,
maintainers can easily confuse flags (such as conflating coarse winding with
top-edge touches) or assume intuitive edge-intersection behavior that fails at
tile boundaries.

## Examples

Bad:

```cpp
struct IntersectionBits {
    // Edge touch and winding bits for tile rasterization:
    static constexpr uint32_t L = 1 << 0;
    static constexpr uint32_t T = 1 << 1;
    static constexpr uint32_t R = 1 << 2;
    static constexpr uint32_t B = 1 << 3;
    static constexpr uint32_t W = 1 << 4;
};
```

Good:

```cpp
struct IntersectionBits {
    // Records which edges a line touches in its tile and whether the tile contributes
    // to coarse winding. Due to corner tie-breaking and differing inclusive/exclusive
    // rules for lines ending on tile boundaries, these do not always represent literal
    // touches. Additionally, coarse winding (W) is related to, but distinct from, top
    // edge touches. These bits serve as a rasterization descriptor, not pure geometry.
    static constexpr uint32_t L = 1 << 0;
    static constexpr uint32_t T = 1 << 1;
    static constexpr uint32_t R = 1 << 2;
    static constexpr uint32_t B = 1 << 3;
    static constexpr uint32_t W = 1 << 4;
};
```
