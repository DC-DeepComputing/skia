---
name: skia/api/dedicated-public-header-for-api-classes
description: When introducing public API classes or extensible interface types, declare them in dedicated headers under include/ rather than inside existing headers or options structs
rubric: design
severity: should_fix
---

**Rule.** Give new public API classes and structs with substantial surface area
their own dedicated public header files under include/ matching the class name,
rather than declaring them inside an existing header. Additionally, define
abstract or virtual interface classes intended for client extension as top-level
types in their own dedicated headers rather than nesting them inside options or
configuration structs, forward-declaring them in the options struct instead.

**Why.** Declaring standalone public classes inside existing headers obscures
API discovery, tightly couples unrelated types, and breaks the 1:1
correspondence between header and source files. Nesting extensible interface
classes inside options structs prevents forward declarations in C++, forcing any
client or compilation unit implementing or referencing the interface to
transitively include the full options header.

## Examples

Bad:

```cpp
// include/gpu/vk/VulkanBackendContext.h
struct SK_API VulkanBackendContext { ... };
class SK_API VulkanPreferredFeatures { ... };

// include/gpu/graphite/ContextOptions.h
struct SK_API ContextOptions {
    class SK_API PersistentCache {
    public:
        virtual ~PersistentCache() = default;
        virtual sk_sp<SkData> load(const SkData& key) = 0;
        virtual void store(const SkData& key, const SkData& data) = 0;
    };
    PersistentCache* fPersistentCache = nullptr;
};
```

Good:

```cpp
// include/gpu/vk/VulkanPreferredFeatures.h
class SK_API VulkanPreferredFeatures { ... };

// include/gpu/graphite/PersistentPipelineStorage.h
namespace skgpu::graphite {
class SK_API PersistentPipelineStorage {
public:
    virtual ~PersistentPipelineStorage() = default;
    virtual sk_sp<SkData> load(const SkData& key) = 0;
    virtual void store(const SkData& key, const SkData& data) = 0;
};
}

// include/gpu/graphite/ContextOptions.h
namespace skgpu::graphite {
class PersistentPipelineStorage;
struct SK_API ContextOptions {
    PersistentPipelineStorage* fPersistentPipelineStorage = nullptr;
};
}
```
