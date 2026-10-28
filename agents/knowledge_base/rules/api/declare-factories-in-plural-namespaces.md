---
name: skia/api/declare-factories-in-plural-namespaces
description: When declaring public primitive factories in headers or docs, use established plural namespaces (e.g., SkShaders, SkImages, SkSurfaces) instead of classes or single-purpose namespaces
rubric: design
severity: should_fix
---

**Rule.** When declaring public factory functions for core primitives (such as
shaders, images, surfaces, or codecs) in headers, place them in the established
plural namespace (such as SkShaders::, SkImages::, SkSurfaces::, or SkCodecs::)
rather than defining static Make* methods on non-constructable classes or
introducing new single-purpose namespaces. Because C++ namespaces are open
across files, modular and utility factories should extend the established plural
namespace in their own header. In documentation, guides, and Markdown code
samples, reference creation functions using the plural namespace (e.g.,
SkSurfaces::WrapPixels) rather than legacy singular class names (e.g.,
SkSurface::).

**Why.** Skia centralizes primitive creation in plural namespaces to maintain a
uniform, modern factory API across modular headers. Creating one-off namespaces
or static class methods fragments API discovery and contradicts the open
namespace design, while referencing legacy singular class names in uncompiled
documentation teaches deprecated patterns that fail when copied into client
code.

## Examples

Bad:

```cpp
// Avoid static Make methods on classes (include/effects/SkGradientShader.h):
class SK_API SkGradientShader {
public:
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const SkGradient&,
                                      const SkMatrix* lm = nullptr);
};

// Avoid introducing single-purpose namespaces for primitive factories:
namespace SkRuntimeEffects {
SK_API SkTracedRuntimeShader MakeTraced(sk_sp<SkShader> shader, const SkIPoint& traceCoord);
}

// In documentation and code samples (docs/**):
sk_sp<SkSurface> surface = SkSurface::WrapPixels(info, &pixelMemory[0], rowBytes);
```

Good:

```cpp
// Extend the established plural namespace across modular headers:
namespace SkShaders {
SK_API sk_sp<SkShader> LinearGradient(const SkPoint pts[2], const SkGradient&,
                                      const SkMatrix* lm = nullptr);

SK_API SkTracedRuntimeShader MakeTracedRuntimeShader(sk_sp<SkShader> shader,
                                                     const SkIPoint& traceCoord);
}

// In documentation and code samples (docs/**):
sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, &pixelMemory[0], rowBytes);
```
