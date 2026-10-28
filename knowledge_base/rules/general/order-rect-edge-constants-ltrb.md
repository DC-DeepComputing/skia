---
name: skia/order-rect-edge-constants-ltrb
description: When defining rectangular or tile edge constants, enum flags, or bitmasks, order them starting with Left (orthodox LTRB or LRTB) rather than vertical-first
rubric: readability
severity: should_fix
---

**Rule.** When defining edge or boundary constants, enum flags, bitmasks, or
struct layouts for rectangles, quads, or tiles, order them starting with Left
(using Skia's orthodox LTRB or LRTB convention matching SkRect and QuadAAFlags)
rather than vertical-first (such as TBLR). When legacy constraints require a
non-standard order, document the rationale and add a TODO to align with LRTB.

**Why.** Skia standardizes rectangular geometry, edge flags, and vector swizzles
around Left-first ordering (LTRB in SkRect and SkSL, and LRTB in QuadAAFlags).
Ordering edge constants or bitmasks vertical-first (T, B, L, R) violates
codebase conventions, creates confusion across coordinate transforms, and risks
mismatch bugs when interacting with standard Skia rect and quad APIs.

## Examples

Bad:

```cpp
struct IntersectionBits {
    // Non-standard vertical-first edge ordering (TBLR):
    static constexpr uint32_t T = 1 << 0;
    static constexpr uint32_t B = 1 << 1;
    static constexpr uint32_t L = 1 << 2;
    static constexpr uint32_t R = 1 << 3;
};
```

Good:

```cpp
struct IntersectionBits {
    // Follow orthodox Skia edge ordering (LTRB, matching SkRect and QuadAAFlags):
    static constexpr uint32_t L = 1 << 0;
    static constexpr uint32_t T = 1 << 1;
    static constexpr uint32_t R = 1 << 2;
    static constexpr uint32_t B = 1 << 3;
};
```
