---
name: skia/tests/test-dedicated-alpha-formats-in-texture-inference
description: When testing texture format and alpha-type inference in Graphite, test dedicated alpha formats across all SkAlphaTypes and verify backend ViewFormat against fallback formats
rubric: testing
severity: should_fix
---

**Rule.** When testing backend texture format and alpha type inference (such as
SkImages::WrapTexture without an explicit color type), include test coverage for
dedicated alpha formats (such as kAlpha_8_SkColorType) across all SkAlphaType
values (kOpaque, kUnknown, kPremul, kUnpremul), not only ambiguous
single-channel formats like kR8_unorm_SkColorType. In backend-parameterized
tests, verify via TextureInfoPriv::ViewFormat that the backend actually created
the requested format rather than an emulated fallback format.

**Why.** Ambiguous single-channel formats map to either red or alpha color types
depending on the specified SkAlphaType, whereas dedicated alpha formats must
strictly map to alpha-only color types regardless of the alpha type provided.
Furthermore, backends that lack native A8 emulate it using R8; checking
ViewFormat avoids redundant test execution on emulated fallbacks.

## Examples

Bad:

```cpp
// Only testing single-channel format ambiguity with R8:
info = caps->getDefaultSampledTextureInfo(kR8_unorm_SkColorType, mipmapped, Protected::kNo, renderable);
texture = recorder->createBackendTexture(kSize, info);
image = SkImages::WrapTexture(recorder.get(), texture, kOpaque_SkAlphaType, nullptr);
REPORTER_ASSERT(reporter, image->colorType() == kR8_unorm_SkColorType);
```

Good:

```cpp
// Test dedicated alpha format (A8) across all alpha types, skipping emulated fallbacks:
info = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType, mipmapped, Protected::kNo, renderable);
if (TextureInfoPriv::ViewFormat(info) != TextureFormat::kA8) {
    continue; // A8 not natively supported on this backend; avoid redundant R8 fallback testing
}
texture = recorder->createBackendTexture(kSize, info);
image = SkImages::WrapTexture(recorder.get(), texture, kOpaque_SkAlphaType, nullptr);
REPORTER_ASSERT(reporter, image->colorType() == kAlpha_8_SkColorType);
REPORTER_ASSERT(reporter, image->alphaType() == kOpaque_SkAlphaType);
```
