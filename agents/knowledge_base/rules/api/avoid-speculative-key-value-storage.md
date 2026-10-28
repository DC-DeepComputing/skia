---
name: skia/api/avoid-speculative-key-value-storage
description: When defining storage or persistence interfaces for a single resource type, tailor the abstraction concretely rather than introducing speculative key-value methods
rubric: design
severity: should_fix
---

**Rule.** Do not design speculative, multi-purpose key-value abstractions (with
arbitrary keys, key-type discriminators, or description strings) for interfaces
that only manage or persist a single concrete resource type. Scope and name the
interface and its member methods concretely for the data handled (e.g.,
dedicated zero-argument load() and single-payload store(data) methods).

**Why.** Speculative key-value interfaces burden callers and implementers with
unnecessary key generation, parameter validation, and hashing contracts when
only one resource blob is ever stored. Tailoring the interface directly to the
concrete resource simplifies client implementations, clarifies API intent, and
eliminates dead abstraction complexity.

## Examples

Bad:

```cpp
// Generic key-value cache with key-type discriminators when only caching pipeline binary blobs
enum class PersistentCacheKeyType : uint32_t {
    kPipelineCache = 1,
};
class SK_API PersistentCache {
public:
    virtual ~PersistentCache() = default;
    virtual sk_sp<SkData> load(const SkData& key) = 0;
    virtual void store(const SkData& key, const SkData& data, const SkString& description) = 0;
};
```

Good:

```cpp
// Scoped concretely to the specific pipeline resource being persisted
class SK_API PersistentPipelineStorage {
public:
    virtual ~PersistentPipelineStorage() = default;
    virtual sk_sp<SkData> load() = 0;
    virtual void store(const SkData& data) = 0;
};
```
