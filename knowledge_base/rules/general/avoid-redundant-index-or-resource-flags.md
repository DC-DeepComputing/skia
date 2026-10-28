---
name: skia/avoid-redundant-index-or-resource-flags
description: When tracking resource, pointer, or index presence in classes, query nullness or use signed sentinels instead of storing redundant boolean flags
rubric: safety
severity: should_fix
---

**Rule.** Do not maintain redundant boolean member variables (such as
fIsIndexed, fHasResource, or fIsLinear) when state is already represented by
whether an existing pointer, container, or resource wrapper is null, empty, or
active. Query the underlying resource or pointer directly (such as checking ptr
== nullptr or evaluating BindBufferInfo), or represent inactive indices using a
signed sentinel value (such as int32_t with -1 or kInvalidIndex).

**Why.** Maintaining separate boolean flags alongside nullable pointers,
containers, or indices creates duplicate state that risks desynchronization and
inflates object layout size with alignment padding. Querying pointer nullness or
resource wrappers directly avoids padding, keeps objects compact
(pointer-sized), enables constexpr default construction, and eliminates
redundant state synchronization.

## Examples

Bad:

```cpp
// Bad: Redundant boolean flags tracking whether an index or resource is present.
class DrawWriter {
    BindBufferInfo fIndices;
    uint32_t fPendingStaticBaseIndices;
    bool fPendingIsIndexed;
};

// Bad: Separate bool flag alongside nullable pointer inflates size and prevents constexpr constructor.
class SkTMaskGamma {
public:
    SkTMaskGamma() : fIsLinear(true) {}
private:
    std::unique_ptr<uint8_t[]> fGammaTables;
    bool fIsLinear;
};
```

Good:

```cpp
// Good: Query resource directly or represent inactive index with a signed sentinel.
class DrawWriter {
    BindBufferInfo fIndices;
    int32_t fPendingStaticBaseIndices = -1; // -1 represents non-indexed / inactive
};

// Good: Query pointer nullness directly; keeps object pointer-sized and allows constexpr construction.
class SkTMaskGamma {
public:
    constexpr SkTMaskGamma() = default;
    constexpr bool isLinear() const { return fGammaTables == nullptr; }
private:
    std::unique_ptr<uint8_t[]> fGammaTables;
};
```
