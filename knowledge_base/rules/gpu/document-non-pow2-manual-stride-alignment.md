---
name: skia/gpu/document-non-pow2-manual-stride-alignment
description: When performing manual ceiling division or stride alignment arithmetic instead of using SkAlignTo, add a comment explaining that strides are non-power-of-two
rubric: readability
severity: nit
---

**Rule.** When computing alignment or ceiling division via manual arithmetic
(such as (bytes + stride - 1) / stride) rather than SkAlignTo, document that
SkAlignTo is intentionally avoided because the stride is not guaranteed to be a
power of two. This clarifies that manual ceiling arithmetic is deliberate and
prevents mistaken refactoring to power-of-two alignment helpers.

**Why.** SkAlignTo requires its alignment argument to be a power of two and
asserts SkIsPow2 in debug builds. When strides (such as arbitrary vertex or
instance attribute sizes) may be non-powers of two, open-coding ceiling division
without an inline comment invites confusion or mistaken refactoring to SkAlignTo
that fails debug assertions.

## Examples

Bad:

```cpp
const size_t prevBytes = fPendingBase * prevStride;
fPendingBase = SkTo<uint32_t>((prevBytes + currStride - 1) / currStride);
```

Good:

```cpp
const size_t prevBytes = fPendingBase * prevStride;
// Not using SkAlignTo because currStride is not guaranteed to be a power of two.
fPendingBase = SkTo<uint32_t>((prevBytes + currStride - 1) / currStride);
```
