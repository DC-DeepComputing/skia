---
name: skia/api/document-mask-coverage-gamma-vs-color-space
description: When declaring gamma or contrast properties on surface or text configuration types, document that they govern mask coverage blending rather than color space
rubric: readability
severity: nit
---

**Rule.** When declaring or modifying gamma or contrast properties on surface or
text configuration types (such as SkSurfaceProps), explicitly document in
comments that the values govern glyph mask coverage blending. Clarify that they
do not configure the destination surface's color space or color channel transfer
functions.

**Why.** In graphics rendering pipelines, gamma can refer to color space
transfer functions on color channels or to mask coverage adjustments used when
blending anti-aliased glyphs. Because surfaces already specify an SkColorSpace,
unqualified gamma or contrast fields in surface configuration structs create
confusion for callers and maintainers about whether they alter color space or
mask blending.

## Examples

Bad:

```cpp
class SkSurfaceProps {
public:
    SkScalar textContrast() const { return fTextContrast; }
    SkScalar textGamma() const { return fTextGamma; }

private:
    uint32_t        fFlags;
    SkPixelGeometry fPixelGeometry;
    SkScalar        fTextContrast;
    SkScalar        fTextGamma;
};
```

Good:

```cpp
class SkSurfaceProps {
public:
    SkScalar textContrast() const { return fTextContrast; }
    SkScalar textGamma() const { return fTextGamma; }

private:
    uint32_t        fFlags;
    SkPixelGeometry fPixelGeometry;

    // This gamma value is specifically about blending of mask coverage.
    // The surface also has a color space, but that applies to the colors.
    SkScalar        fTextContrast;
    SkScalar        fTextGamma;
};
```
