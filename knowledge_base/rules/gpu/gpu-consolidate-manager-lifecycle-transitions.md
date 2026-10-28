---
name: skia/gpu/gpu-consolidate-manager-lifecycle-transitions
description: In Graphite buffer and resource managers, encapsulate post-pass capacity tuning and state resets within existing lifecycle methods rather than exposing separate public cleanup steps
rubric: design
severity: should_fix
---

**Rule.** When designing or updating Graphite resource and buffer managers (such
as DrawBufferManager or UploadBufferManager), encapsulate post-pass tuning,
dynamic capacity adjustments, and state resets within existing one-and-done
lifecycle transition methods (such as transferToRecording()). Do not expose
separate public fine-grained update or cleanup methods that require external
coordinating callers (such as Recorder::snap()) to orchestrate multi-step
lifecycle handoffs.

**Why.** Exposing fine-grained cleanup or tuning methods leaks internal state
management and introduces temporal coupling. Consolidating adjustments into
existing lifecycle boundaries guarantees that capacity updates and state resets
execute reliably without risking callers omitting necessary post-pass
adjustments.

## Examples

Bad:

```cpp
// BufferManager.h:
class DrawBufferManager {
public:
    [[nodiscard]] bool transferToRecording(Recording*);
    void updateStorageBufferBlockSize();
};

// Recorder.cpp:
bool valid = fDrawBufferManager->transferToRecording(recording.get());
fDrawBufferManager->updateStorageBufferBlockSize();
```

Good:

```cpp
// BufferManager.cpp:
bool DrawBufferManager::transferToRecording(Recording* recording) {
    // Encapsulate block size tuning and state reset inside lifecycle transition:
    const uint32_t reqSize = SkAlignTo(info.fUsedSize + info.fOffset, info.fMinBlockSize);
    info.fCurBlockSize = std::clamp(reqSize, info.fMinBlockSize, info.fMaxBlockSize);
    info.fUsedSize = 0;
    // ... transfer buffers to recording ...
    return true;
}
```
