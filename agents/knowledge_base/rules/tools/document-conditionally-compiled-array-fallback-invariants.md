---
name: skia/tools/document-conditionally-compiled-array-fallback-invariants
description: When selecting a fallback option by indexing into a conditionally compiled array, document the non-empty invariant and why the chosen element is adequate
rubric: readability
severity: nit
---

**Rule.** When selecting a fallback configuration or backend by indexing into an
array populated via conditional compilation directives (such as
kSupportedBackends[0]), add an inline comment documenting that the array is
guaranteed to be non-empty. Explicitly explain why picking the indexed entry
(such as the first element) provides an adequate fallback.

**Why.** Unchecked indexing into an array whose elements are conditionally
compiled can look like an out-of-bounds hazard or an arbitrary choice.
Documenting the non-emptiness invariant and ordering rationale clarifies why the
fallback is safe across all build configurations.

## Examples

Bad:

```cpp
#if defined(SK_GANESH) && defined(SK_GL)
    Window::BackendType windowTypeForRaster = Window::kNativeGL_BackendType;
#else
    Window::BackendType windowTypeForRaster = kSupportedBackends[0];
#endif
```

Good:

```cpp
#if defined(SK_GANESH) && defined(SK_GL)
    Window::BackendType windowTypeForRaster = Window::kNativeGL_BackendType;
#else
    // kSupportedBackends will always have at least one entry. Picking the first should
    // usually result in an adequate GPU-backend.
    Window::BackendType windowTypeForRaster = kSupportedBackends[0];
#endif
```
