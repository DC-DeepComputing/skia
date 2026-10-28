---
name: skia/gpu/distinguish-resource-from-global-capability-queries
description: When adding resource-specific capability queries to Caps or GPU classes, use distinct names rather than overloading parameterless global queries to avoid base-class member hiding
rubric: readability
severity: nit
---

**Rule.** Do not overload capability query methods across a GPU class hierarchy
when one method checks global backend support and another checks per-resource
support (such as for a specific TextureInfo or format). Give the
resource-specific query a distinct, descriptive name (e.g.,
isRenderableWithMSRTSS or msaaTextureRenderToSingleSampledSupport rather than
overloading msaaRenderToSingleSampledSupport) to prevent call-site ambiguity and
avoid base-class member hiding that requires 'using Base::method;' declarations.

**Why.** Overloading a parameterless global capability check with a
resource-parameterized query creates semantic ambiguity at call sites over
whether global hardware support or target-specific compatibility is being
verified. Furthermore, in C++, overriding an overloaded virtual function in a
derived class hides all base class overloads with the same name, forcing awkward
'using Base::method;' declarations to restore member lookup.

## Examples

Bad:

```cpp
// In Caps.h
bool msaaRenderToSingleSampledSupport() const;
virtual bool msaaRenderToSingleSampledSupport(const TextureInfo& info) const;

// In VulkanCaps.h: Derived override hides base zero-argument overload
class VulkanCaps : public Caps {
    using Caps::msaaRenderToSingleSampledSupport;
    bool msaaRenderToSingleSampledSupport(const TextureInfo& info) const override;
};
```

Good:

```cpp
// In Caps.h: Global capability and resource queries have distinct names
bool msaaRenderToSingleSampledSupport() const;
virtual bool isRenderableWithMSRTSS(const TextureInfo& info) const;

// In VulkanCaps.h: Clean override without name hiding or 'using' declarations
class VulkanCaps : public Caps {
    bool isRenderableWithMSRTSS(const TextureInfo& info) const override;
};
```
