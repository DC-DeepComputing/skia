---
name: skia/tests/use-kopaque-for-graphite-kunknown-alpha-reference
description: When generating reference or expected SkImageInfo for Graphite textures wrapped with kUnknown_SkAlphaType, set the alpha type to kOpaque_SkAlphaType rather than kUnpremul_SkAlphaType
rubric: correctness
severity: should_fix
---

**Rule.** When generating reference pixel data or expected SkImageInfo for
Graphite textures wrapped with kUnknown_SkAlphaType, set the alpha type to
kOpaque_SkAlphaType rather than kUnpremul_SkAlphaType. Align expected readback
pixmaps with Graphite's forced-opaque behavior when comparing readback results.

**Why.** Graphite forces wrapped textures with kUnknown_SkAlphaType to opaque by
inserting an RGB1 swizzle and reporting kOpaque_SkAlphaType. Generating
reference data with kUnpremul_SkAlphaType contradicts the runtime alpha type and
swizzling behavior, leading to false mismatch assertions during readback
verification.

## Examples

Bad:

```cpp
} else if (srcAT == kUnknown_SkAlphaType) {
    // Regenerating expected data as unpremul contradicts forced-opaque behavior.
    srcInfo = srcInfo.makeAlphaType(kUnpremul_SkAlphaType);
    srcStorage.alloc(srcInfo);
    refPixels = make_ref_data(refInfo, /*forceOpaque=*/true);
    refPixels.readPixels(srcStorage, 0, 0);
}
```

Good:

```cpp
} else if (srcAT == kUnknown_SkAlphaType) {
    // Graphite forces kUnknown_SkAlphaType to opaque on readback.
    srcInfo = srcInfo.makeAlphaType(kOpaque_SkAlphaType);
    srcStorage.alloc(srcInfo);
    refPixels = make_ref_data(refInfo, /*forceOpaque=*/true);
    refPixels.readPixels(srcStorage, 0, 0);
}
```
