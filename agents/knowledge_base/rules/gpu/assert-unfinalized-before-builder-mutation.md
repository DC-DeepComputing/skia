---
name: skia/gpu/assert-unfinalized-before-builder-mutation
description: In GPU builder and accumulator classes, assert unfinalized state with SkASSERT(!this->isFinalized()) in mutating and finalize methods instead of defensive warning logs
rubric: readability
severity: should_fix
---

**Rule.** In GPU builder, gatherer, and accumulator classes with distinct
accumulation and finalization phases, assert that the object has not been
finalized (e.g., SkASSERT(!this->isFinalized()) or SkASSERT(!fFinalized)) upon
entry to methods that allocate, append, or mutate data, as well as in
finalize(). Do not defensively handle double-finalization or lifecycle contract
violations with runtime warning logs (such as SKGPU_LOG_W) and early returns.

**Why.** Mutating or re-finalizing an accumulator after finalization corrupts
buffer offsets, leaks resources, or desynchronizes state. Handling lifecycle
contract violations defensively with runtime warning logs and early returns
masks caller bugs in production builds, whereas asserting unfinalized state
ensures state machine violations fail fast in debug builds and tests.

## Examples

Bad:

```cpp
// Bad: Mutating without verifying unfinalized state
std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    if (int* existingOffset = fGradientOffsetCache.find(shader->uniqueID())) {
        return {nullptr, *existingOffset};
    }
    return this->allocateFloatData(numStops * 5);
}

// Bad: Defensively logging a warning and early returning on double finalization
void StorageManager::finalize(DrawBufferManager* bufferMgr) {
    if (fIsFinalized) {
        SKGPU_LOG_W("Warning: attempting to finalize an already finalized storage manager!");
        return;
    }
    // ...
}
```

Good:

```cpp
// Good: Assert unfinalized state upon entry before mutating state
std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    SkASSERT(!this->isFinalized());
    if (int* existingOffset = fGradientOffsetCache.find(shader->uniqueID())) {
        return {nullptr, *existingOffset};
    }
    return this->allocateFloatData(numStops * 5);
}

// Good: Assert lifecycle contract directly in finalize()
void StorageManager::finalize(DrawBufferManager* bufferMgr) {
    SkASSERT(!this->isFinalized());
    // ...
}
```
