---
name: skia/modules/use-drawimagerect-for-svg-image-bounds
description: When rendering images into explicit target bounds or viewports in SVG elements, use SkCanvas::drawImageRect instead of SkCanvas::drawImage to ensure proper scaling
rubric: correctness
severity: should_fix
---

**Rule.** When rendering an image into a bounding box with explicit target
dimensions in SVG nodes (such as <image>), use SkCanvas::drawImageRect with the
destination rectangle rather than SkCanvas::drawImage. Ensure the image scales
to fit the specified bounds instead of rendering at intrinsic pixel dimensions.

**Why.** SkCanvas::drawImage(image, x, y) renders the image at its intrinsic
pixel dimensions without scaling. SVG element attributes define destination
viewport dimensions (x, y, width, height) that require scaling the image to fit
the destination bounds via SkCanvas::drawImageRect.

## Examples

Bad:

```cpp
ctx.canvas()->drawImage(image, rect.fLeft, rect.fTop);
```

Good:

```cpp
ctx.canvas()->drawImageRect(image, rect, SkSamplingOptions(SkFilterMode::kLinear));
```
