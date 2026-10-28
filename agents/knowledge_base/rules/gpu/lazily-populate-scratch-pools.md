---
name: skia/gpu/lazily-populate-scratch-pools
description: When managing scratch helper or builder pools and recursion stacks, allocate instances lazily on demand rather than eagerly pre-allocating during initialization
rubric: safety
severity: should_fix
---

**Rule.** Populate scratch helper pools, builder caches, and recursion stacks
lazily on demand rather than eagerly pre-allocating elements up to maximum
capacity during initialization. Have acquisition helpers (such as
popOrCreate...) check whether the pool is empty, popping an existing instance or
allocating a new one dynamically, and return instances to the pool upon release
up to the maximum capacity bound.

**Why.** Eagerly pre-allocating multiple scratch objects or recursion helpers
during object construction wastes memory and incurs allocation overhead at
startup. In common execution paths, operations rarely exceed minimal recursion
depths (typically depth 1), so allocating instances on demand optimizes
common-case performance while bounding peak capacity.

## Examples

Bad:

```cpp
// Eagerly pre-populating helper pool up to capacity in constructor:
Recorder::Recorder(...) {
    for (int i = 0; i < kMaxKeyAndDataBuilders; ++i) {
        fKeyAndDataBuilders.push_back(make_key_and_data_builder(...));
    }
}
```

Good:

```cpp
// Pool starts empty; instances are allocated on demand:
std::unique_ptr<KeyAndDataBuilder> RecorderPriv::popOrCreateKeyAndDataBuilder() {
    if (!fRecorder->fKeyAndDataBuilders.empty()) {
        std::unique_ptr<KeyAndDataBuilder> builder = std::move(fRecorder->fKeyAndDataBuilders.back());
        fRecorder->fKeyAndDataBuilders.pop_back();
        return builder;
    }
    return make_key_and_data_builder(...);
}
```
