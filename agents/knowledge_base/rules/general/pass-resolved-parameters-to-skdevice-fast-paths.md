---
name: skia/pass-resolved-parameters-to-skdevice-fast-paths
description: When adding SkDevice virtual fast-path methods, name them after the geometry and pass resolved effect parameters rather than unpacking filters in backends
rubric: design
severity: should_fix
---

**Rule.** In SkDevice virtual fast-path methods (such as analytic blurs), name
methods after the drawn geometry (e.g., drawBlurredRRect) and pass pre-resolved,
matrix-transformed effect parameters directly from SkCanvas. Do not name
virtuals after SkPaint filter types (e.g., drawRectBlurMaskFilter) or require
backend devices to unpack and transform paint filters.

**Why.** SkCanvas coordinates paint hierarchy and transformation matrices,
whereas SkDevice abstracts concrete backend rasterization. Forcing backends to
unpack filters and recalculate CTM-transformed parameters duplicates validation
and transformation logic across every backend device implementation.

## Examples

Bad:

```cpp
// In SkDevice.h:
virtual bool drawRectBlurMaskFilter(const SkRect&, const SkPaint&) { return false; }

// In backend Device.cpp:
bool Device::drawRectBlurMaskFilter(const SkRect& rect, const SkPaint& paint) {
    const SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (!mf || mf->type() != SkMaskFilterBase::Type::kBlur) return false;
    float transformedSigma = static_cast<const SkBlurMaskFilterImpl*>(mf)->computeXformedSigma(this->localToDeviceTransform());
    ...
}
```

Good:

```cpp
// In SkDevice.h:
virtual bool drawBlurredRRect(const SkRRect&, const SkPaint&, float transformedSigma) {
    return false;
}

// In SkCanvas.cpp:
if (const SkBlurMaskFilterImpl* blur = can_attempt_blurred_rrect_draw(paint)) {
    float transformedSigma = blur->computeXformedSigma(this->getTotalMatrix());
    if (this->topDevice()->drawBlurredRRect(rrect, layer->paint(), transformedSigma)) {
        return;
    }
}
```
