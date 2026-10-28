---
name: skia/name-descriptor-parameters-desc
description: When declaring SkDescriptor or SkFontDescriptor parameters or variables, name them desc rather than descr to maintain codebase consistency
rubric: readability
severity: nit
---

**Rule.** Name SkDescriptor and SkFontDescriptor parameters and local variables
desc rather than descr or verbose alternatives. Maintain this naming consistency
across font interfaces, scaler context methods (such as onCreateScalerContext),
and typeface implementations.

**Why.** Across Skia's font subsystem and scaler context interfaces, descriptor
pointers are consistently named desc. Deviating to descr breaks consistency with
base virtual method signatures and peer typeface implementations, while also
adding unnecessary characters to already long parameter lists.

## Examples

Bad:

```cpp
std::unique_ptr<SkScalerContext> SkTypeface_proxy::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* descr) const {
    return this->onCreateScalerContextAsProxyTypeface(effects, descr, nullptr);
}
```

Good:

```cpp
std::unique_ptr<SkScalerContext> SkTypeface_proxy::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return this->onCreateScalerContextAsProxyTypeface(effects, desc, nullptr);
}
```
