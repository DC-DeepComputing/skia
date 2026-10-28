---
name: skia/gpu/check-dirty-flag-before-deferred-sync
description: When managing dirty-tracking flags for deferred GPU synchronization or persistent cache storage, check the flag before syncing and reset it upon completion
rubric: correctness
severity: should_fix
---

**Rule.** When implementing deferred state synchronization, flushes, or cache
persistence guarded by dirty-tracking flags, explicitly verify the dirty flag
before executing updates or driver calls rather than synchronizing
unconditionally. Additionally, upon storing, flushing, or synchronizing, always
reset the dirty or change flag to false—including in feature fallback modes
where precise mutation detection is unavailable—re-marking the flag dirty when
new resources or modifications occur.

**Why.** Omitting the dirty check before synchronization causes expensive driver
calls or string formatting to run when state is unchanged. Conversely, failing
to reset dirty flags upon persistence—such as under heuristic or fallback
detection modes—leaves the flag permanently stuck at true, causing callers to
repeatedly perform redundant flushes or storage writes.

## Examples

Bad:

```cpp
// 1. Missing dirty check before executing deferred sync
void synchronizeBackendLabel() {
    if (!fLabel.empty()) {
        this->setBackendLabel(("Skia_" + fLabel).c_str());
    }
    fBackendLabelDirty = false;
}

// 2. Skipping dirty flag reset on store in fallback mode
void storePipelineCacheData(size_t maxSize) {
    this->resourceProvider().storePipelineCacheData(maxSize);
    if (!this->canDetectNewPipelineCacheData()) {
        return; // Dirty flag remains permanently true
    }
    fHasNewPipelineCacheData = false;
}
```

Good:

```cpp
// 1. Verify dirty flag before executing deferred sync
void synchronizeBackendLabel() {
    if (fBackendLabelDirty && !fLabel.empty()) {
        this->setBackendLabel(("Skia_" + fLabel).c_str());
    }
    fBackendLabelDirty = false;
}

// 2. Always reset dirty flag on store; fallback re-flags on creation
void storePipelineCacheData(size_t maxSize) {
    this->resourceProvider().storePipelineCacheData(maxSize);
    fHasNewPipelineCacheData = false;
}
```
