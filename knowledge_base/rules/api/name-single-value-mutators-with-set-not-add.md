---
name: skia/api/name-single-value-mutators-with-set-not-add
description: When assigning or replacing a single stored instance rather than accumulating into a collection or registry, use set or Set instead of add or Register
rubric: readability
severity: should_fix
---

**Rule.** When declaring methods or functions that assign or overwrite a single
stored instance or member variable rather than appending to a collection or
registry, name them with set or Set (e.g., setStaticVertexBuffer, SetFactory) to
reflect replacement semantics. Reserve add for appending to collections and
Register for multi-provider registries or open collections.

**Why.** In C++ APIs, prefixes like add and Register suggest accumulating
elements into an open collection or multi-provider registry. When a function or
method actually overwrites or assigns a single stored instance, naming it add or
Register misleads callers into expecting accumulation semantics and can hide
bugs where repeated calls unintentionally overwrite previous values. Using
set/Set clearly signals single-value mutation and pairs symmetrically with
getters.

## Examples

Bad:

```cpp
// Overwrites single stored member on each call:
void addStaticVertexResource(const BindBufferInfo& info, sk_sp<Resource> resource) {
    fStaticVertexInfo = info;
    fStaticVertexBuffer = resource;
}

// Replaces a single global factory instance:
SK_API sk_sp<Factory> GetFactory();
SK_API void RegisterFactory(sk_sp<Factory>);
```

Good:

```cpp
// Clearly conveys single-value replacement:
void setStaticVertexBuffer(const BindBufferInfo& info, sk_sp<Resource> resource) {
    fStaticVertexInfo = info;
    fStaticVertexBuffer = resource;
}

// Symmetrical setter for single-instance replacement:
SK_API const sk_sp<Factory>& GetFactory();
SK_API void SetFactory(sk_sp<Factory>);
```
