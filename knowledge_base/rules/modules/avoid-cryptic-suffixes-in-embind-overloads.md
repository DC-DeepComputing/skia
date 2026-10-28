---
name: skia/modules/avoid-cryptic-suffixes-in-embind-overloads
description: When registering Emscripten embind function overloads in CanvasKit, bind them under identical names using select_overload instead of adding cryptic suffixes
rubric: readability
severity: nit
---

**Rule.** When binding overloaded C++ functions or methods in Emscripten
bindings (EMSCRIPTEN_BINDINGS), bind overloads under the exact same JavaScript
identifier using embind's select_overload<...>. Do not append cryptic or
arbitrary single-letter suffixes (such as 'P' or '2') to differentiate
signatures, and only use distinct names if the methods represent fundamentally
different operations.

**Why.** Emscripten's embind supports signature-based function overloading
natively via select_overload, allowing multiple C++ overloads to share the same
JavaScript method name. Appending arbitrary single-letter suffixes clutters the
binding surface, obscures function intent, and creates confusing, unidiomatic
APIs for JavaScript callers.

## Examples

Bad:

```cpp
EMSCRIPTEN_BINDINGS(Skia) {
    function("_MakeOnScreenGLSurface",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int, sk_sp<SkColorSpace>)>(
                 &MakeOnScreenGLSurface));
    function("_MakeOnScreenGLSurfaceP",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int, sk_sp<SkColorSpace>, int, int)>(
                 &MakeOnScreenGLSurface));
}
```

Good:

```cpp
EMSCRIPTEN_BINDINGS(Skia) {
    function("_MakeOnScreenGLSurface",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int, sk_sp<SkColorSpace>)>(
                 &MakeOnScreenGLSurface));
    function("_MakeOnScreenGLSurface",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int, sk_sp<SkColorSpace>, int, int)>(
                 &MakeOnScreenGLSurface));
}
```
