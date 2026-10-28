---
name: skia/core/avoid-synthetic-draw-for-blitter-selection
description: When selecting CPU blitters in standalone helpers, invoke chooser functions directly with explicit parameters and SkBlitterSizedArena instead of synthesizing dummy SkDraw contexts
rubric: design
severity: should_fix
---

**Rule.** Do not construct dummy or synthetic Draw or SkDraw objects solely to
choose a blitter via SkAutoBlitterChoose in standalone rasterization helpers.
Invoke blitter chooser functions (such as SkChooseA8Blitter or
SkBlitter::Choose) directly with explicit parameters and allocate blitters using
an SkBlitterSizedArena.

**Why.** SkDraw is designed as an execution context orchestrating full primitive
draws dispatched from SkBitmapDevice. Synthesizing dummy SkDraw instances with
placeholder clips, matrices, or chooser pointers adds misleading boilerplate and
couples utility functions to device draw harnesses when only blitter allocation
is required.

## Examples

Bad:

```cpp
Draw draw;
draw.fRC = &clip;
draw.fCTM = &translate;
draw.fBlitterChooser = SkA8Blitter_Choose;
if (!draw.fDst.reset(mask)) {
    return;
}
SkPaint paint;
paint.setAntiAlias(true);
SkAutoBlitterChoose blitterChooser(draw, nullptr, paint, SkRect::Make(draw.fDst.bounds()));
SkBlitter* blitter = blitterChooser.get();
```

Good:

```cpp
SkPixmap dst;
if (!dst.reset(mask)) {
    return;
}
SkPaint paint;
paint.setAntiAlias(true);
SkBlitterSizedArena alloc;
SkBlitter* blitter = SkChooseA8Blitter(dst, translate, paint, alloc.arena(),
                                      SkDrawCoverage::kNo, nullptr);
```
