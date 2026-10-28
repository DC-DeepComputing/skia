---
name: skia/omit-redundant-container-bounds-assertions
description: When indexing into Skia collections like TArray, STArray, or SkSpan, omit redundant bounds assertions or manual if-checks since operator[] already validates bounds
rubric: readability
severity: should_fix
---

**Rule.** When indexing into Skia collections like skia_private::TArray,
STArray, SkTDArray, or SkSpan, do not add redundant bounds assertions (such as
SkASSERT(i < array.size())) or manual bounds checks (such as if (i >= 0 && i <
array.size())) unless out-of-bounds access is an explicit API contract meant to
be tolerated as a fallback. Rely on operator[], which already performs bounds
validation via sk_collection_check_bounds in debug builds.

**Why.** Skia's core collection types implement subscript bounds checking
directly inside operator[] using sk_collection_check_bounds, aborting on
out-of-bounds indices in debug builds. Adding explicit pre-indexing assertions
duplicates existing checks, while adding defensive if-bounds guards in accessors
adds branching boilerplate and can mask upstream caller bugs.

## Examples

Bad:

```cpp
// Redundant debug assertion:
const VulkanSampler* immutableSampler(int i) const {
    SkASSERT(i >= 0 && i < fImmutableSamplers.size());
    return fImmutableSamplers[i].get();
}

// Redundant defensive bounds check in getter:
sk_sp<SkPicture> getAsset(int i) const {
    if (i >= 0 && i < fAssets.size()) {
        return fAssets[i];
    }
    return nullptr;
}
```

Good:

```cpp
const VulkanSampler* immutableSampler(int i) const {
    return fImmutableSamplers[i].get();
}

sk_sp<SkPicture> getAsset(int i) const {
    return fAssets[i];
}
```
