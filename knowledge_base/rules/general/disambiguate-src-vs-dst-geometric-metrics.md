---
name: skia/disambiguate-src-vs-dst-geometric-metrics
description: When declaring geometric metrics, bounds, or subset operations across coordinate spaces, qualify method and variable identifiers with their explicit space or storage layer
rubric: readability
severity: should_fix
---

**Rule.** When declaring geometric metrics, insets, offsets, bounds, filter
parameters, or subsetting methods spanning multiple coordinate spaces,
explicitly qualify identifier and method names with their concrete space or
storage layer (such as 'device'/'dev' vs 'local', 'src' vs 'dst', or
'BackingStore' vs logical view bounds). Avoid unqualified names (e.g.,
'padding', 'bounds', or 'onMakeSubset') or ambiguous descriptors like
'transformed' that obscure the target coordinate frame or misrepresent
containment invariants.

**Why.** Rendering, atlas, and imaging pipelines routinely mix values across
coordinate frames (such as local geometry, device surfaces, texture sources, and
backing storage). Ambiguous names like 'transformedSigma' or unqualified method
and variable names like 'padding', 'bounds', or 'onMakeSubset' obscure which
coordinate space applies, leading callers and backend implementations to make
wrong assumptions, violate containment bounds, or double-transform geometry.

## Examples

Bad:

```cpp
// Ambiguous: unclear whether padding applies to texture source or target bounds
virtual int glyphPadding() const = 0;
virtual SkRect glyphBounds() const = 0;

// Ambiguous descriptor: fails to specify target coordinate space
const float transformedSigma = blurMaskFilter->computeXformedSigma(this->getTotalMatrix());
virtual bool drawBlurredRRect(const SkRRect&, const SkPaint&, float transformedSigma);

// Misleading: implies subset of this special image's view/content bounds rather than backing storage
virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;
```

Good:

```cpp
// Explicit: clarifies coordinates apply to source atlas vs destination space
virtual int glyphSrcPadding() const = 0;
virtual SkRect glyphDstBounds() const = 0;

// Explicit coordinate space: clarifies sigma is in device coordinate space
const float deviceSigma = blurMaskFilter->computeXformedSigma(this->getTotalMatrix());
virtual bool drawBlurredRRect(const SkRRect&, const SkPaint&, float deviceSigma);

// Explicit storage layer: clarifies coordinates are in backing store space and not constrained to image subset
virtual sk_sp<SkSpecialImage> onMakeBackingStoreSubset(const SkIRect& subset) const = 0;
```
