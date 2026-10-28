---
name: skia/api/name-role-delegating-factories-as-not-with
description: When declaring factory or creation methods where the created object assumes a delegate or proxy role, use As<Role> rather than With<Role>
rubric: readability
severity: nit
---

**Rule.** When naming factory or creation methods where an object is constructed
to act on behalf of or assume the identity of another entity (such as a proxy,
wrapper, or delegate), use 'As<Role>' (e.g.,
onCreateScalerContextAsProxyTypeface) rather than 'With<Role>'. Reserve
'With<Role>' for methods accepting secondary configuration parameters,
dependencies, or auxiliary inputs rather than defining role delegation.

**Why.** Prepositions in API names establish semantic relationships: 'With'
indicates an auxiliary configuration or secondary dependency passed to assist
the creation of the host object, whereas 'As' signifies role assumption or
identity delegation where the resulting object acts on behalf of or masquerades
as the designated entity. Misusing 'With' misleads callers into thinking the
parameter is a subordinate helper rather than the primary delegated identity.

## Examples

Bad:

```cpp
// Misleading: 'With' implies the proxy typeface is merely an auxiliary input.
virtual std::unique_ptr<SkScalerContext> onCreateScalerContextWithProxyTypeface(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkTypeface* proxyTypeface) const;
```

Good:

```cpp
// Clear: 'As' indicates the scaler context is created to act as the proxy typeface.
virtual std::unique_ptr<SkScalerContext> onCreateScalerContextAsProxyTypeface(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkTypeface* proxyTypeface) const;
```
