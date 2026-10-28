---
name: skia/document-kunknown-alpha-wraptexture-opaque
description: When passing kUnknown_SkAlphaType to SkImages::WrapTexture to force an RGB1 swizzle and treat the texture as opaque, document the intent with an inline comment
rubric: readability
severity: nit
---

**Rule.** When passing kUnknown_SkAlphaType to SkImages::WrapTexture (or related
Graphite texture wrapping helpers) to force an RGB1 swizzle and treat the
texture as opaque, add an inline explanatory comment (e.g., '// force alpha
channel to 1' or '// force opaque'). Clarify the non-obvious intent so readers
and reviewers do not mistake the unknown alpha type for uninitialized or
placeholder data.

**Why.** In Graphite, kUnknown_SkAlphaType instructs texture sampling to force
the alpha channel to 1 via an RGB1 swizzle and reports kOpaque_SkAlphaType.
Because kUnknown_SkAlphaType conventionally implies missing or uninitialized
alpha metadata, call sites must explicitly document that the value intentionally
forces opaque sampling.

## Examples

Bad:

```cpp
fYCbCrImage = SkImages::WrapTexture(recorder,
                                    ycbcrHelper->backendTexture(),
                                    kUnknown_SkAlphaType,
                                    /*colorSpace=*/nullptr,
                                    release_ycbcrhelper,
                                    ycbcrHelper.get());
```

Good:

```cpp
fYCbCrImage = SkImages::WrapTexture(recorder,
                                    ycbcrHelper->backendTexture(),
                                    kUnknown_SkAlphaType, // force alpha channel to 1
                                    /*colorSpace=*/nullptr,
                                    release_ycbcrhelper,
                                    ycbcrHelper.get());
```
