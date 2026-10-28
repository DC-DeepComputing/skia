---
name: skia/avoid-static-inline-in-anonymous-namespaces
description: When defining file-local helper functions in C++ source files or tests, omit redundant inline on static helpers and omit both inside anonymous namespaces
rubric: readability
severity: should_fix
---

**Rule.** Do not mark functions or variables defined inside an anonymous
namespace (namespace { ... }) in C++ source files with 'static' or 'inline',
defining them as plain non-static members. Additionally, do not mark file-local
static helper functions in .cpp files or unit tests as 'inline' unless justified
by performance measurement.

**Why.** Symbols defined in an anonymous namespace already possess
translation-unit internal linkage, making 'static' redundant. Furthermore,
compilers automatically evaluate inlining for file-local functions based on
optimization heuristics, making manual 'inline' annotations on static helpers or
in test code unnecessary noise unless supported by profiling measurements.

## Examples

Bad:

```cpp
namespace {

static inline SkColorType renderable_colortype(SkColorType srcCT, SkColorType dstCT) {
    return dstCT != kUnknown_SkColorType ? dstCT : srcCT;
}

static bool is_supported_format(TextureFormat format) {
    return format != TextureFormat::kUnsupported;
}

}  // namespace

// File-local static helper in .cpp or test:
static inline sk_sp<SkSurface> create_test_surface() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(70, 70);
    return SkSurfaces::Raster(info);
}
```

Good:

```cpp
namespace {

SkColorType renderable_colortype(SkColorType srcCT, SkColorType dstCT) {
    return dstCT != kUnknown_SkColorType ? dstCT : srcCT;
}

bool is_supported_format(TextureFormat format) {
    return format != TextureFormat::kUnsupported;
}

}  // namespace

// File-local static helper in .cpp or test:
static sk_sp<SkSurface> create_test_surface() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(70, 70);
    return SkSurfaces::Raster(info);
}
```
