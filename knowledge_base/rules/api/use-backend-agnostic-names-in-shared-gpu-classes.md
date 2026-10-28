---
name: skia/api/use-backend-agnostic-names-in-shared-gpu-classes
description: In shared GPU interfaces and Caps, name methods and capability queries after generic semantic effects rather than backend prefixes or specific API extensions
rubric: readability
severity: should_fix
---

**Rule.** In shared GPU abstractions, base classes, and public interfaces (such
as Context, Recorder, Texture, or Caps), use generic, backend-agnostic
terminology (such as isTransient) and omit backend-specific prefixes (e.g., Vk,
GL, Mtl, Dawn) or graphics API extension names (e.g., Dawn's
ExpandResolveTexture) from method, parameter, and type names. Name capability
queries and methods after their generic semantic effect or invariant (such as
loadOpAffectsMSAAPipelines() or storePipelineCacheData()) rather than
backend-specific mechanisms (such as supportsExpandResolveTextureLoadOp() or
storeVkPipelineCacheData()). Additionally, when declaring queries or predicates
that inspect descriptor usage flags (such as
VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT), name them after the usage flag (e.g.,
has_transient_usage()) rather than backing memory properties (such as
isMemoryless()).

**Why.** Shared GPU public interfaces and capability abstractions define common
contracts across graphics APIs. Adding backend prefixes, backend-specific memory
concepts, or proprietary extension names leaks driver details into generic
interfaces, causes friction when other backends implement the capability, and
obscures the semantic invariant required by callers.

## Examples

Bad:

```cpp
// In shared include/gpu/graphite/Context.h:
bool hasNewVkPipelineCacheData() const;
void storeVkPipelineCacheData(size_t maxSize);

// In cross-backend src/gpu/graphite/Caps.h:
virtual bool supportsExpandResolveTextureLoadOp() const { return false; }

// In shared src/gpu/graphite/Texture.h:
Texture(const SharedContext*, SkISize dimensions, const TextureInfo& info,
        bool usesLazyMemory, sk_sp<MutableTextureState>, Ownership);

// In backend descriptor query:
bool isMemoryless() const {
    return fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
}
```

Good:

```cpp
// In shared include/gpu/graphite/Context.h:
bool hasNewPipelineCacheData() const;
void storePipelineCacheData(size_t maxSize);

// In cross-backend src/gpu/graphite/Caps.h:
virtual bool loadOpAffectsMSAAPipelines() const { return false; }

// In shared src/gpu/graphite/Texture.h:
Texture(const SharedContext*, SkISize dimensions, const TextureInfo& info,
        bool isTransient, sk_sp<MutableTextureState>, Ownership);

// In backend descriptor query:
static bool has_transient_usage(const TextureInfo& info) {
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);
    return vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
}
```
