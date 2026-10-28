---
name: skia/tests/forward-coordinate-parameters-to-pixmap-accessors
description: When defining test helper functions that accept coordinate parameters for pixel access or assertion, forward them into underlying accessors rather than hardcoding (0, 0)
rubric: correctness
severity: should_fix
---

**Rule.** When defining test helper functions that accept coordinate or offset
parameters (such as x and y) for pixel inspection, sampling, or assertions,
forward those arguments into the underlying accessor (such as
SkPixmap::getColor(x, y) or SkPixmap::addr32(x, y)) instead of hardcoding
constant offsets like (0, 0).

**Why.** Hardcoding constant coordinate offsets such as (0, 0) inside test
helpers ignores caller-provided coordinates. When callers pass non-zero
coordinates (or when helpers declare default coordinates), tests silently
inspect (0, 0) regardless of the requested coordinates, producing false-positive
passes and masking decoding, rasterization, or sampling regressions.

## Examples

Bad:

```cpp
// Variant 1: Ignores x and y in raw pixel accessor
static std::optional<uint32_t> decodeSinglePixel(skiatest::Reporter* r, const char* path, int x = 0, int y = 0) {
    ...
    return *pixmap.addr32(0, 0);
}

// Variant 2: Ignores coordinate parameters in color assertion helper
void AssertPixel(skiatest::Reporter* r, const SkPixmap& pixmap, int x, int y) {
    SkColor color = pixmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SK_ColorGREEN);
}
```

Good:

```cpp
// Variant 1: Forwards coordinates to raw pixel accessor
static std::optional<uint32_t> decodeSinglePixel(skiatest::Reporter* r, const char* path, int x = 0, int y = 0) {
    ...
    return *pixmap.addr32(x, y);
}

// Variant 2: Forwards coordinates to color lookup
void AssertPixel(skiatest::Reporter* r, const SkPixmap& pixmap, int x, int y) {
    SkColor color = pixmap.getColor(x, y);
    REPORTER_ASSERT(r, color == SK_ColorGREEN);
}
```
