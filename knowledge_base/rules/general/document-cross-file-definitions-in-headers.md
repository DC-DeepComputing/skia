---
name: skia/document-cross-file-definitions-in-headers
description: When declaring functions or methods in headers that are defined in a different .cpp file, document their definition location and rationale
rubric: readability
severity: nit
---

**Rule.** When a class member function or free function declared in a header is
defined in a different translation unit rather than its matching <Class>.cpp or
<Header>.cpp file, document where it is defined and why at the declaration.
Clarify which .cpp file contains the definition and the reason for placing it
there (such as sharing internal helper functions or avoiding duplicate logic
across subsystems).

**Why.** Developers expect member functions to be implemented in their
corresponding .cpp source file. When an implementation lives in a different
translation unit to access internal helpers or consolidate shared logic,
documenting the target file and rationale directly at the declaration prevents
confusion and eases code navigation.

## Examples

Bad:

```cpp
// In SkEmbossMaskFilter.h
class SkEmbossMaskFilter : public SkMaskFilterBase {
public:
    // Defined across translation units without documenting where or why
    static sk_sp<SkImageFilter> LegacySpecular(const SkPoint3& direction, SkColor lightColor,
                                               SkScalar surfaceScale, SkScalar ks,
                                               SkScalar shininess, sk_sp<SkImageFilter> input);
};
```

Good:

```cpp
// In SkEmbossMaskFilter.h
class SkEmbossMaskFilter : public SkMaskFilterBase {
public:
    // Defined in SkLightingImageFilter.cpp because it overlaps heavily with
    // SkImageFilters::DistantLitSpecular and shares internal lighting logic.
    static sk_sp<SkImageFilter> LegacySpecular(const SkPoint3& direction, SkColor lightColor,
                                               SkScalar surfaceScale, SkScalar ks,
                                               SkScalar shininess, sk_sp<SkImageFilter> input);
};
```
