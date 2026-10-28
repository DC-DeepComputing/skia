---
name: skia/core/use-destination-load-stages-in-raster-pipeline
description: When assembling SkRasterPipeline destination stages or appending to appendLoadDst, use destination load stage variants (*_dst) rather than source load stages (load_*)
rubric: correctness
severity: should_fix
---

**Rule.** When assembling destination stages in SkRasterPipeline (such as within
SkRasterPipeline::appendLoadDst or custom blitter pipeline builders), append
destination load stage variants (such as Op::load_*_dst) rather than standard
source load stages (Op::load_*).

**Why.** Standard load stages write pixel data into the active source color
registers (r, g, b, a), whereas *_dst stage variants load into destination
registers (dr, dg, db, da). Appending a source load inside destination loading
helpers clobbers the active source color and fails to supply the destination
color needed for blending.

## Examples

Bad:

```cpp
case kR16_float_SkColorType:
    this->append(Op::load_rf16, ctx);
    break;
```

Good:

```cpp
case kR16_float_SkColorType:
    this->append(Op::load_rf16_dst, ctx);
    break;
```
