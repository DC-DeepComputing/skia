---
name: skia/gpu/record-high-water-mark-at-finalization
description: When tracking buffer high-water marks for capacity management in Graphite builders, update peak sizes upon finalization or locking rather than in intermediate append or alignment methods
rubric: correctness
severity: should_fix
---

**Rule.** When managing dynamically sized buffers or builders that track storage
high-water marks for capacity management (such as PaintParamsKeyBuilder or
UniformManager), update the high-water mark only during finalization or locking
methods (such as lockAsKey() or finish()). Do not record or update high-water
marks inside intermediate append, write, alignment, or padding helpers.

**Why.** Intermediate append, write, or alignment helpers only observe buffer
size at a single step, failing to account for structural elements added earlier
or later or builders that omit specific data types. Updating high-water marks
upon finalization ensures peak capacity is measured accurately before
capacity-shrinking checks and eliminates redundant calculations during
intermediate operations.

## Examples

Bad:

```cpp
void PaintParamsKeyBuilder::addData(SkSpan<const uint32_t> data) {
    fData.push_back(data.size());
    fData.push_back_n(data.size(), data.begin());
    fDataHighWaterMark = std::max(fDataHighWaterMark, fData.size());
}

void UniformManager::alignForNonShading(int requiredAlignment) {
    this->alignTo(requiredAlignment);
    fNonShadingOffset = fStorage.size();
    fStorageHighWaterMark = std::max(fStorageHighWaterMark, fStorage.size());
}
```

Good:

```cpp
void PaintParamsKeyBuilder::addData(SkSpan<const uint32_t> data) {
    fData.push_back(data.size());
    fData.push_back_n(data.size(), data.begin());
}
PaintParamsKey PaintParamsKeyBuilder::lockAsKey() {
    fDataHighWaterMark = std::max(fDataHighWaterMark, fData.size());
    return PaintParamsKey({fData.data(), fData.size()});
}

void UniformManager::alignForNonShading(int requiredAlignment) {
    this->alignTo(requiredAlignment);
    fNonShadingOffset = fStorage.size();
}
```
