---
name: skia/ports/ports-distinguish-real-and-proxy-typefaces
description: In font backend wrappers and typeface delegation interfaces, name the underlying font engine typeface realTypeface and delegating wrapper proxyTypeface rather than ambiguously proxy
rubric: readability
severity: should_fix
---

**Rule.** In font backend wrappers, typeface delegation, and scaler context
creation (such as FreeType, Fontations, and FontConfig), name parameters and
variables representing the underlying font engine typeface 'realTypeface' and
those representing the delegating wrapper 'proxyTypeface'. Do not name either
side ambiguously as 'proxy' or mislabel the underlying typeface as a proxy.

**Why.** When font managers delegate to an underlying font engine, calling both
sides 'proxy' or mislabeling the underlying typeface as a proxy obscures which
object manages native font engine resources and which interacts with the strike
cache and client callers. Explicit naming clarifies ownership, lifetime, and
dispatch roles across font subsystem interfaces.

## Examples

Bad:

```cpp
// Ambiguous: naming the delegating wrapper or the underlying typeface 'proxy' obscures roles.
std::unique_ptr<SkScalerContext> SkTypeface_FreeType::onCreateScalerContextAsProxyTypeface(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkTypeface* proxy) const {
    return std::make_unique<SkScalerContext_FreeType>(
            *this,
            effects,
            desc,
            proxy ? *proxy : *const_cast<SkTypeface_FreeType*>(this));
}
```

Good:

```cpp
// Explicit: 'proxyTypeface' identifies the delegating wrapper; 'realTypeface' identifies the engine typeface.
std::unique_ptr<SkScalerContext> SkTypeface_FreeType::onCreateScalerContextAsProxyTypeface(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkTypeface* proxyTypeface) const {
    return std::make_unique<SkScalerContext_FreeType>(
            *this,  // realTypeface
            effects,
            desc,
            proxyTypeface ? *proxyTypeface : *const_cast<SkTypeface_FreeType*>(this));
}
```
