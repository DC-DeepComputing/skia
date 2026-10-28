---
name: skia/codec/encapsulate-swizzler-scratch-buffer-allocation
description: When allocating intermediate color transformation scratch buffers in image decoders, encapsulate buffer allocation inside initializeSwizzler rather than duplicating checks across decode entry points
rubric: design
severity: should_fix
---

**Rule.** In image decoders, encapsulate intermediate scratch buffer allocations
and condition checks (such as allocating fXformBuffer when xformOnDecode() is
true) directly inside initializeSwizzler(). Do not duplicate scratch buffer
sizing or allocation logic across individual decode entry points (such as
onGetPixels, onStartScanlineDecode, or onStartIncrementalDecode).

**Why.** Multiple decode entry points require identical swizzler and scratch
buffer configurations when color transformation on decode is enabled.
Centralizing buffer allocation inside initializeSwizzler eliminates repetitive
boilerplate across call sites and prevents buffer omission bugs when introducing
new decode entry points.

## Examples

Bad:

```cpp
// Duplicated in onGetPixels() and onStartScanlineDecode():
if (this->xformOnDecode()) {
    fXformBuffer.reset(new uint32_t[info.width()]);
}
return this->initializeSwizzler(info, options);
```

Good:

```cpp
// Inside initializeSwizzler():
if (this->xformOnDecode()) {
    fXformBuffer.reset(new uint32_t[dstInfo.width()]);
    swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
}
// Callers simply invoke initializeSwizzler():
return this->initializeSwizzler(info, options);
```
