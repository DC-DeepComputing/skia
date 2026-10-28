---
name: skia/use-atomics-for-worker-thread-flags
description: When managing cross-thread state flags, use std::atomic and exchange() for concurrent access, but use plain types and document lifecycle invariants when access is serialized
rubric: safety
severity: should_fix
---

**Rule.** Use atomic types (such as std::atomic<bool> or std::atomic<int>) or
explicit synchronization primitives for state flags and counters in shared
managers, contexts, or caches accessed across multiple threads, and detect
compound transitions using exchange() alongside a mutex for accompanying shared
data. Conversely, do not use std::atomic for isolated member flags when state
access is already serialized by lifecycle or reference-ownership invariants; use
plain types instead and document the non-overlapping execution phases or
ownership conditions that guarantee thread safety.

**Why.** Concurrent read and write operations on plain, non-atomic shared
variables across threads constitute data races, causing undefined behavior and
memory incoherency, while un-synchronized compound transitions cause
check-then-act races. Conversely, declaring isolated member variables as
std::atomic when access is already serialized by lifecycle or ownership
invariants adds unnecessary overhead and misleadingly suggests concurrent access
to the overall object is safe.

## Examples

Bad:

```cpp
// Non-atomic field toggled across threads causes data races:
class SkCaptureManager : public SkRefCnt {
    bool fIsCurrentlyCapturing = false;
};

// Check-then-act race and unguarded shared state despite atomic flag:
void SkCaptureManager::toggleCapture(bool capturing) {
    if (capturing != fIsCurrentlyCapturing && !capturing) {
        fPictures.clear();
    }
    fIsCurrentlyCapturing = capturing;
}

// Misleading atomic flag when access is serialized by lifecycle invariants:
class Resource {
    std::string fLabel;
    std::atomic<bool> fBackendLabelDirty;
};
```

Good:

```cpp
// Thread-safe atomic flag prevents data races across threads:
class SkCaptureManager : public SkRefCnt {
    std::atomic<bool> fIsCurrentlyCapturing = false;
};

// Atomic exchange detects transition; mutex protects shared mutable state:
void SkCaptureManager::toggleCapture(bool capturing) {
    bool wasCapturing = fIsCurrentlyCapturing.exchange(capturing);
    if (wasCapturing && !capturing) {
        SkAutoMutexExclusive lock(fMutex);
        fPictures.clear();
    }
}

// Plain bool with documented non-overlapping lifecycle invariants:
class Resource {
    std::string fLabel;
    // Unsynchronized: only accessed when the caller holds sole ownership or
    // during non-overlapping Context insertion phases.
    bool fBackendLabelDirty = false;
};
```
