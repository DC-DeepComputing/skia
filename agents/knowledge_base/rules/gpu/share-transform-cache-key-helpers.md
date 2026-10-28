---
name: skia/gpu/share-transform-cache-key-helpers
description: When generating UniqueKey cache keys for atlas or mask entries with transforms, use shared key helpers instead of duplicating matrix decomposition
rubric: design
severity: should_fix
---

**Rule.** When generating `UniqueKey` cache keys for atlas or mask entries,
factor transform serialization and subpixel translation extraction into shared
utilities such as `GenerateClipMaskKey`, `GeneratePathMaskKey`, or
`add_transform_key`. Do not open-code matrix decomposition (scale, skew,
translation fractions) into `UniqueKey::Builder` across individual atlas
managers.

**Why.** Atlas managers caching entries based on affine transforms and subpixel
translations share identical key requirements. Duplicating matrix decomposition
and bit-packing logic across managers causes code bloat and cache key format
drift.

## Examples

Bad:

```cpp
// In ClipAtlasManager.cpp:
SkMatrix mat = (*elementsForMask)[i]->fLocalToDevice.matrix().asM33();
SkScalar sx = mat.get(SkMatrix::kMScaleX);
SkScalar sy = mat.get(SkMatrix::kMScaleY);
SkScalar tx = mat.get(SkMatrix::kMTransX);
SkScalar ty = mat.get(SkMatrix::kMTransY);
SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
builder[elementKeyIndex + 0] = SkFloat2Bits(sx);
builder[elementKeyIndex + 1] = SkFloat2Bits(sy);
```

Good:

```cpp
// In ClipAtlasManager.cpp:
skgpu::UniqueKey maskKey = GenerateClipMaskKey(stackRecordID,
                                               elementList,
                                               maskDeviceBounds,
                                               requiredPathCount);
```
