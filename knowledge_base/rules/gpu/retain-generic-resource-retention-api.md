---
name: skia/gpu/retain-generic-resource-retention-api
description: When managing static resource lifetimes in shared GPU caches, retain generic retention interfaces like addStaticResource instead of specializing for specific buffer types
rubric: design
severity: should_fix
---

**Rule.** Do not replace or narrow general-purpose resource retention APIs (such
as addStaticResource(sk_sp<Resource>)) with domain-specific methods tailored to
individual consumers or asset types. Keep the lifetime-retention interface
general so arbitrary static GPU assets (including static buffers, lookup tables,
and dither textures) can be held uniformly.

**Why.** Shared caches and lifetime managers must retain diverse immutable
resources for the lifetime of a GPU context. Specializing retention methods for
specific buffer bindings couples the cache interface to particular render stages
and prevents storing other resource types uniformly.

## Examples

Bad:

```cpp
// Replacing generic retention with type-specific setters:
class GlobalCache {
public:
    void addStaticVertexBuffer(sk_sp<Buffer>);
    void addStaticIndexBuffer(sk_sp<Buffer>);
};
```

Good:

```cpp
// Preserving generic resource retention for any static asset:
class GlobalCache {
public:
    void addStaticResource(sk_sp<Resource>);
};
```
