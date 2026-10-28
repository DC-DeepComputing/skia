---
name: skia/gpu/include-dstout-in-blend-dst-dependency-checks
description: When analyzing Porter-Duff blend modes for destination color dependence, include SkBlendMode::kDstOut alongside kSrcOver and avoid speculative TODOs for additional modes
rubric: readability
severity: should_fix
---

**Rule.** When checking whether a Porter-Duff blend mode depends on destination
color (e.g. to determine whether destination reads can be avoided or
pixel-overwrite optimizations apply), include SkBlendMode::kDstOut alongside
SkBlendMode::kSrcOver. Both modes use (1 - Sa) as the destination blend
coefficient and become completely independent of destination color when the
source is opaque. Do not add speculative TODO comments questioning whether
additional modes avoid destination reads: only kClear, kSrc, and opaque
kSrcOver/kDstOut are mathematically independent of the destination.

**Why.** In Porter-Duff blending, kDstOut computes d * (1 - Sa). When the source
is opaque (Sa == 1), the destination factor zeroes out and the blend simplifies
to kClear, making output pixels independent of destination color. All other
blend modes mathematically reference destination color or alpha. Omitting
kDstOut causes unnecessary destination reads or assertion failures, while
speculative TODO comments questioning if other modes exist are inaccurate and
misleading.

## Examples

Bad:

```cpp
bool blendmode_depends_on_dst(SkBlendMode blendMode, bool srcIsOpaque) {
    if (blendMode == SkBlendMode::kSrc || blendMode == SkBlendMode::kClear) {
        return false;
    }
    // Bad: assumes only kSrcOver avoids dst reads when opaque, and speculates on others.
    if (blendMode == SkBlendMode::kSrcOver) {
        return !srcIsOpaque;
    }
    // TODO: Are there other modes that don't depend on dst that can be trivially detected?
    return true;
}
```

Good:

```cpp
bool blendmode_depends_on_dst(SkBlendMode blendMode, bool srcIsOpaque) {
    if (blendMode == SkBlendMode::kSrc || blendMode == SkBlendMode::kClear) {
        return false;
    }
    // Good: kDstOut also zeroes destination factor (1 - Sa) when opaque.
    // Mathematically, no other blend modes avoid reading destination.
    if (blendMode == SkBlendMode::kSrcOver || blendMode == SkBlendMode::kDstOut) {
        return !srcIsOpaque;
    }
    return true;
}
```
