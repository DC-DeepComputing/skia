---
name: skia/ports/distinguish-real-from-proxy-parameters
description: When passing or constructing typeface and scaler context proxy wrappers, name parameters explicitly as realTypeface and proxyTypeface rather than generic identifiers
rubric: readability
severity: should_fix
---

**Rule.** When a function or constructor accepts both an underlying
implementation object and an outer wrapper or proxy (such as in SkTypeface_proxy
or SkScalerContext implementations), name parameters symmetrically as real<Type>
and proxy<Type> (e.g., realTypeface and proxyTypeface). Do not use asymmetric or
generic names like bare typeface alongside proxy.

**Why.** In proxy and wrapper patterns, both the wrapped implementation and the
wrapper instance share the same base type or domain role. Generic names like
typeface alongside proxy obscure which parameter supplies the underlying
data/implementation and which represents the outer delegate, complicating role
clarity and ownership tracking.

## Examples

Bad:

```cpp
SkScalerContext_FreeType::SkScalerContext_FreeType(sk_sp<SkTypeface_FreeType> typeface,
                                                   const SkScalerContextEffects& effects,
                                                   const SkDescriptor* desc,
                                                   sk_sp<SkTypeface> proxy)
    : SkScalerContext(proxy, effects, desc)
{
    fFaceRec = typeface->getFaceRec();
}
```

Good:

```cpp
SkScalerContext_FreeType::SkScalerContext_FreeType(const SkTypeface_FreeType& realTypeface,
                                                   const SkScalerContextEffects& effects,
                                                   const SkDescriptor* desc,
                                                   SkTypeface& proxyTypeface)
    : SkScalerContext(proxyTypeface, effects, desc)
{
    fFaceRec = realTypeface.getFaceRec();
}
```
