---
name: skia/pdf/parameterize-color-space-conversion-helpers
description: When serializing color spaces or ICC profiles in PDF generation, reuse and parameterize shared profile helpers instead of inlining multi-step profile conversion pipelines
rubric: design
severity: should_fix
---

**Rule.** When serializing image or document color spaces into PDF ICC profiles,
reuse and parameterize existing shared conversion helpers (such as
GetColorSpaceAsIccProfile) rather than duplicating the multi-step profile export
sequence inline. Parameterize shared helpers to accept caller-specific
parameters (such as channel counts or document pointers) instead of
re-implementing toProfile and SkWriteICCProfile pipelines across call sites.

**Why.** Duplicating multi-step color space extraction and ICC serialization
sequences inline increases maintenance overhead and leads to code drift when
profile formatting or error-handling requirements change. Centralizing the
pipeline in a parameterized helper preserves consistency across image and stream
embedding paths.

## Examples

Bad:

```cpp
skcms_ICCProfile imageIccProfile;
imageColorSpace->toProfile(&imageIccProfile);
sk_sp<SkData> imageIccData = SkWriteICCProfile(&imageIccProfile, "");
colorSpace = write_icc_profile(doc, std::move(imageIccData), channels);
```

Good:

```cpp
colorSpace = GetColorSpaceAsIccProfile(doc, imageColorSpace, channels);
```
