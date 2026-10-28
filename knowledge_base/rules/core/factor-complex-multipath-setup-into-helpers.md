---
name: skia/core/factor-complex-multipath-setup-into-helpers
description: When coordinating complex multi-path layer or canvas setup across multiple filter conditions, factor distinct branches into dedicated private helpers sharing common setup
rubric: design
severity: should_fix
---

**Rule.** When a class constructor or setup method coordinates multiple complex,
distinct execution paths (such as handling different image filter or mask filter
configurations), factor each branch into a dedicated private helper method and
extract shared initialization logic into a common helper rather than inlining
all branches into a monolithic function body.

**Why.** Inlining multiple complex setup paths into a single method body makes
control flow difficult to trace, inflates function length, and obscures
branch-specific preconditions. Decomposing distinct paths into private helper
methods chunks the method logically and avoids duplicating common layer setup or
bounds calculation boilerplate.

## Examples

Bad:

```cpp
// Inlining multiple complex filter setup paths and duplicating layer creation in constructor
AutoLayerForImageFilter::AutoLayerForImageFilter(SkCanvas* canvas, const SkPaint& paint, const SkRect* rawBounds) {
    if (fPaint.getImageFilter() && !SkCanvasPriv::ImageToColorFilter(&fPaint)) {
        // Multi-line image filter bounds calculation, paint cloning, and canvas->internalSaveLayer()...
    }
    if (fPaint.getMaskFilter()) {
        // Multi-line mask filter bounds calculation, paint cloning, and canvas->internalSaveLayer()...
    }
}
```

Good:

```cpp
// Factoring branches into private helpers and sharing common layer creation logic
void AutoLayerForImageFilter::addImageFilterLayer(const SkRect* drawBounds) {
    // Branch-specific setup...
    this->addLayer(restorePaint, drawBounds, /*coverageOnly=*/false);
}
void AutoLayerForImageFilter::addMaskFilterLayer(const SkRect* drawBounds) {
    // Branch-specific setup...
    this->addLayer(restorePaint, drawBounds, /*coverageOnly=*/!appliesShading);
}
AutoLayerForImageFilter::AutoLayerForImageFilter(SkCanvas* canvas, const SkPaint& paint, const SkRect* rawBounds) {
    if (fPaint.getImageFilter() && !SkCanvasPriv::ImageToColorFilter(&fPaint)) {
        this->addImageFilterLayer(rawBounds);
    }
    if (fPaint.getMaskFilter()) {
        this->addMaskFilterLayer(rawBounds);
    }
}
```
