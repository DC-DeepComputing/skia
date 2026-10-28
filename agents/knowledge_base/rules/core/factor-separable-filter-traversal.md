---
name: skia/core/factor-separable-filter-traversal
description: When implementing separable 2D image filters or blurs, abstract 1D scanline processing behind a pass interface rather than duplicating 2D traversal loops
rubric: design
severity: should_fix
---

**Rule.** In multi-pass or separable 2D image filtering (such as CPU raster
blurs), separate high-level image orchestration—including bounds inflation,
intermediate buffer allocation, and row/column coordinate traversal—from 1D
scanline pixel processing. Abstract 1D scanline work behind a dedicated pass
interface (such as a Pass or PassMaker subclass) evaluated by a shared
multi-pass traversal driver rather than duplicating nested loops across pixel
formats.

**Why.** Separable 2D CPU filters share identical outer coordinate calculation
and buffer traversal mechanics across data types. Abstracting scanline
processing into a pass interface decouples algorithmic mathematics from image
iteration, eliminating redundant allocation and looping boilerplate while
preventing subtle coordination divergence between formats.

## Examples

Bad:

```cpp
// Bad: Duplicating image buffer allocation, bounds inflation, and nested traversal loops across pixel formats
SkBitmap dst;
dstBounds.outset(borderW, borderH);
dst.tryAllocPixels(imageInfo);
for (int y = loopStart; y < loopEnd; ++y) {
    // 1D horizontal pass logic duplicated inline
}
for (int x = loopStart; x < loopEnd; ++x) {
    // 1D vertical pass logic duplicated inline
}
```

Good:

```cpp
// Good: Abstract 1D scanline work behind Pass subclasses and delegate outer traversal to a shared driver
PassMaker* makerX = makeMaker(sigma.width());
PassMaker* makerY = makeMaker(sigma.height());
return eval_blur_passes<uint8_t>(makerX, makerY, src, originalSrcBounds, originalDstBounds, &alloc);
```
