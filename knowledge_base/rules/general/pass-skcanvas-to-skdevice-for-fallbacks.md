---
name: skia/pass-skcanvas-to-skdevice-for-fallbacks
description: When SkDevice rendering methods need canvas-level features or re-entrant draws for fallbacks, pass SkCanvas* to co-locate fallback handling with the primary draw logic
rubric: design
severity: should_fix
---

**Rule.** When SkDevice rendering operations have fast-path fallbacks that
require canvas-level features (such as mask filters, blurs, or simplified
draws), pass SkCanvas* to the SkDevice method rather than returning fallback
flags to SkCanvas. Keep the fallback drawing and paint-construction logic
co-located within the SkDevice implementation instead of scattering fallback
handling across SkCanvas.

**Why.** Different backends (such as Graphite and Ganesh) handle features like
mask filters or tessellation fallbacks at different layers. Passing SkCanvas*
into SkDevice allows fallback and feature-construction logic to remain
encapsulated alongside the primary drawing calculations, avoiding leaking
backend-specific fallback branches and paint setup into shared frontend SkCanvas
dispatch.

## Examples

Bad:

```cpp
// In SkCanvas::onDrawShadowRec:
auto [handledAmbient, handledSpot] = this->topDevice()->drawShadow(path, rec);
if (!handledAmbient) {
    // SkCanvas implements blur fallback and mask filter construction
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, false));
    this->drawPath(devSpacePath, paint);
}
```

Good:

```cpp
// In SkCanvas::onDrawShadowRec:
this->topDevice()->drawShadow(this, path, rec);

// In SkDevice / SkShadowUtils.cpp:
void SkDevice::drawShadow(SkCanvas* canvas, const SkPath& path, const SkDrawShadowRec& rec) {
    if (!this->drawFastPath(path, rec)) {
        // Fallback blur stays co-located with shadow calculation logic
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, false));
        canvas->drawPath(devSpacePath, paint);
    }
}
```
