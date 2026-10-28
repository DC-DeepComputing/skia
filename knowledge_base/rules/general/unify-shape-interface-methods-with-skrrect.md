---
name: skia/unify-shape-interface-methods-with-skrrect
description: When adding device or backend interface methods for standard shapes, unify rect, oval, and rounded rect operations under SkRRect instead of proliferating separate virtuals
rubric: design
severity: should_fix
---

**Rule.** Do not proliferate separate virtual methods on SkDevice or backend
interfaces for rectangles, circles/ovals, and rounded rectangles. Unify them
under a single interface method accepting SkRRect, and let backend
implementations inspect SkRRect::getType() to dispatch to specialized analytic
fast-paths internally when needed.

**Why.** SkRRect can canonically represent rects (kRect_Type), ovals and circles
(kOval_Type), and rounded rects (kSimple_Type, etc.). Adding separate virtual
methods multiplies interface boilerplate across all SkDevice subclasses (such as
bitmap, Ganesh, Graphite, and PDF) and often artificially restricts
functionality (such as supporting only circles rather than arbitrary ovals),
whereas SkRRect allows frontends to normalize geometry and backends to share
common setup before dispatching to shape-specific fast-paths.

## Examples

Bad:

```cpp
// Proliferating separate virtual methods for each shape type across SkDevice subclasses
virtual bool drawRectBlurMaskFilter(const SkRect&, const SkPaint&) { return false; }
virtual bool drawRRectBlurMaskFilter(const SkRRect&, const SkPaint&) { return false; }
virtual bool drawCircleBlurMaskFilter(const SkRect&, const SkPaint&) { return false; }
```

Good:

```cpp
// Unified virtual method accepting SkRRect for rects, ovals, and rounded rects
virtual bool drawBlurredRRect(const SkRRect& rrect, const SkPaint& paint, float transformedSigma) {
    return false;
}
```
