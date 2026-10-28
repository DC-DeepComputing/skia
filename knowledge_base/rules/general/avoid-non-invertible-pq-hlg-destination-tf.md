---
name: skia/avoid-non-invertible-pq-hlg-destination-tf
description: When configuring destination color spaces for skcms_Transform, do not use non-invertible kPQ or kHLG transfer functions; use invertible PQish or HLGish representations instead
rubric: correctness
severity: blocking
---

**Rule.** Do not use kPQ or kHLG (or transfer functions classified as
skcms_TFType_PQ / skcms_TFType_HLG) directly as destination color spaces or
profiles passed to skcms_Transform. Because skcms_TransferFunction_invert cannot
invert standard PQ or HLG transfer functions and returns false, map them to
invertible PQish or HLGish parametric representations or invert them via
dedicated intermediate steps.

**Why.** skcms_Transform requires inverting the destination profile's transfer
function to convert from XYZ/linear space to destination device RGB. When
skcms_TransferFunction_invert() encounters skcms_TFType_PQ or skcms_TFType_HLG,
it returns false, causing skcms_Transform() to fail and triggering fatal
assertion failures (such as SkAssertResult in SkCodec::applyColorXform).

## Examples

Bad:

```cpp
// Setting destination profile to standard PQ or HLG transfer functions:
skcms_ICCProfile dstProfile;
skcms_Init(&dstProfile);
dstProfile.has_trc = true;
dstProfile.trc[0].parametric = SkNamedTransferFn::kPQ;

// skcms_TransferFunction_invert returns false for skcms_TFType_PQ,
// causing skcms_Transform to fail:
SkAssertResult(skcms_Transform(src, srcFmt, srcAlpha, &srcProfile,
                               dst, dstFmt, dstAlpha, &dstProfile, count));
```

Good:

```cpp
// Map PQ or HLG to invertible PQish/HLGish transfer functions for skcms destinations:
skcms_ICCProfile dstProfile;
skcms_Init(&dstProfile);
dstProfile.has_trc = true;

// Use invertible PQish parametric representation:
static constexpr skcms_TransferFunction kPQish =
    {-2.0f, -107/128.0f, 1.0f, 32/2523.0f, 2413/128.0f, -2392/128.0f, 8192/1305.0f};
dstProfile.trc[0].parametric = kPQish;

SkAssertResult(skcms_Transform(src, srcFmt, srcAlpha, &srcProfile,
                               dst, dstFmt, dstAlpha, &dstProfile, count));
```
