---
name: skia/prefer-file-local-static-over-class-methods
description: When defining internal, single-use, or debug helper functions, define them as file-static or anonymous-namespace helpers in .cpp files instead of header member functions
rubric: design
severity: should_fix
---

**Rule.** Do not declare translation-unit-local, single-use, or debug helper
functions as member functions or inline helpers in class headers, especially
when guarded by conditional compilation (#ifdef SK_DEBUG, #ifdef GPU_TEST_UTILS)
or implementable via existing class accessors without needing private class
state. For straightforward single-use operations or API wrappers, inline the
call directly at the call site in the .cpp file. For non-trivial or debug
verification helpers used within a single translation unit, define them as
file-local static functions (using lower_case_with_underscores naming) or in an
anonymous namespace in the .cpp implementation file.

**Why.** Declaring internal or debug helpers in class headers clutters class
interfaces, exposes internal implementation or verification details, leaks
compilation guards, and triggers unnecessary recompilation across translation
units when modified. Keeping helpers file-static or in anonymous namespaces in
.cpp files preserves encapsulation and prevents header compilation dependencies.

## Examples

Bad:

```cpp
// In SkSVGFilterContext.h:
class SkSVGFilterContext {
    ...
private:
    static sk_sp<SkImageFilter> ConvertFilterColorspace(sk_sp<SkImageFilter>&&,
                                                        SkSVGColorspace,
                                                        SkSVGColorspace);
};

// In SkPathData.h:
class SkPathData {
    ...
private:
    static SkPathData* PeekEmptySingleton();
};

#ifdef SK_DEBUG
    void checkDimensionsExpection(SkISize expected) const;
#endif
```

Good:

```cpp
// In SkSVGFilterContext.cpp:
namespace {
sk_sp<SkImageFilter> ConvertFilterColorspace(sk_sp<SkImageFilter>&& input,
                                             SkSVGColorspace src,
                                             SkSVGColorspace dst) {
    ...
}
}  // namespace

// In SkPathData.cpp (file-local static helper):
static SkPathData* peek_empty_singleton() {
    static SkPathData* gEmpty = ...;
    return gEmpty;
}

#ifdef SK_DEBUG
static void check_dimensions_match_target(const GrSurfaceProxy* proxy, SkISize expected) {
    ...
}
#endif
```
