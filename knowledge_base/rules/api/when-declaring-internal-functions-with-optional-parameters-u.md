---
name: skia/api/when-declaring-internal-functions-with-optional-parameters-u
description: When declaring functions or constructors with optional pointer or smart pointer (sk_sp) parameters, provide default arguments (= nullptr) to document optionality and simplify call sites
rubric: readability
severity: blocking
---

**Rule.** When declaring functions, helper methods, or constructors with
optional pointer or smart pointer (sk_sp<T>) parameters that are unused across
most call sites, supply default parameter arguments (= nullptr) in the
declaration. Omit explicit null arguments at typical call sites rather than
forcing callers to pass boilerplate nullptr literals.

**Why.** Defaulting optional pointer and smart pointer parameters to nullptr
documents at the API interface that null is valid and supported when unused. It
eliminates call-site boilerplate and visual clutter by relieving callers that do
not require the optional dependency from passing repetitive explicit nullptr
arguments.

## Examples

Bad:

```cpp
// Bad: Forcing callers to pass nullptr for rarely-used optional parameters
static sk_sp<SkImage_Raster> MakeFromBitmap(const SkBitmap&,
                                            SkCopyPixelsMode,
                                            sk_sp<SkMipmap>);

// Caller:
return SkImage_Raster::MakeFromBitmap(fBitmap, cpm, nullptr);

// Bad: Requiring callers to pass nullptr for optional constructor dependencies
SlotManager(const SkString, sk_sp<skresources::ResourceProvider>,
            sk_sp<skottie::PropertyObserver>);
```

Good:

```cpp
// Good: Defaulting optional parameters keeps call sites clean
static sk_sp<SkImage_Raster> MakeFromBitmap(const SkBitmap&,
                                            SkCopyPixelsMode,
                                            sk_sp<SkMipmap> = nullptr);

// Caller:
return SkImage_Raster::MakeFromBitmap(fBitmap, cpm);

// Good: Defaulting optional dependencies to nullptr documents optionality
SlotManager(const SkString, sk_sp<skresources::ResourceProvider>,
            sk_sp<skottie::PropertyObserver> = nullptr);
```
