---
name: skia/api/document-backend-invariants-on-subclasses
description: When documenting virtual methods across polymorphic hierarchies, describe portable semantic contracts on base classes rather than subclass- or backend-specific behavior
rubric: readability
severity: should_fix
---

**Rule.** When documenting virtual methods or interfaces across polymorphic
hierarchies, describe the general semantic contract and caller intent rather
than implementation-specific side effects or guarantees that only hold for a
single subclass (such as SkBitmapDevice). Keep platform- or backend-specific
driver invariants, image layouts, and implementation TODOs anchored on concrete
subclasses or overrides (such as VulkanTexture or SkTypeface_fontconfig) rather
than in backend-agnostic base classes or generic proxies.

**Why.** Base class interface and virtual method comments establish expectations
across all current and future implementations; stating guarantees that hold only
for one subclass misleads callers and implementers of other subclasses.
Furthermore, leaking backend-specific driver invariants, layouts, or TODOs into
generic base classes violates architectural layering and clutters portable
abstractions with backend-specific technical debt.

## Examples

Bad:

```cpp
// Variant 1: Base class comment stating a single subclass's side effect
// In src/core/SkDevice.h (base class):
// Drawing operations will not function after this is called.
virtual void setImmutable() {}

// Variant 2: Backend-specific driver invariant in generic base class
// In src/gpu/graphite/Texture.h (generic GPU base class):
// Attempts to write to texture on host. In Vulkan, the image will be in
// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout at the end.
virtual bool uploadDataOnHost(const UploadSource& source);

// Variant 3: Backend-specific TODO in generic proxy class
// In src/ports/SkTypeface_proxy.cpp (generic proxy class):
sk_sp<SkTypeface> SkTypeface_proxy::onMakeClone(const SkFontArguments& args) const {
    // TODO: need to clone FC_MATRIX and FC_EMBOLDEN by wrapping this
    return fProxy->onMakeClone(args);
}
```

Good:

```cpp
// Variant 1: Describe semantic contract and caller intent on base class
// In src/core/SkDevice.h (base class):
// Marking an SkDevice immutable declares the intent that rendering is complete,
// allowing sampling without copying. Drawing operations may not function and may assert.
virtual void setImmutable() {}

// Variant 2: Move backend-specific invariant to concrete subclass override
// In src/gpu/graphite/Texture.h (generic base class):
virtual bool uploadDataOnHost(const UploadSource& source);
// In src/gpu/graphite/vk/VulkanTexture.h (concrete subclass):
// On completion, image is in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout.
bool uploadDataOnHost(const UploadSource& source) override;

// Variant 3: Keep backend TODO in concrete subclass override
// In src/ports/SkFontMgr_fontconfig.cpp (concrete subclass):
sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
    // TODO: need to clone FC_MATRIX and FC_EMBOLDEN by wrapping this
    return SkTypeface_proxy::onMakeClone(args);
}
```
