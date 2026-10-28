---
name: skia/api/gpu-configure-context-callbacks-via-options
description: When adding callbacks to GPU contexts, supply lifetime callbacks via ContextOptions and attach workload completion procs via parameter structs like SubmitInfo
rubric: design
severity: should_fix
---

**Rule.** Place context-level configuration and callbacks that should be
immutable for the context's lifetime into ContextOptions (Graphite) or
GrContextOptions (Ganesh) rather than exposing mutable setter methods on Context
or GrDirectContext. Similarly, attach workload completion callbacks (such as
GpuFinishedProc) via operation parameter structs (like SubmitInfo or
InsertRecordingInfo) rather than adding standalone callback-registration methods
to Context, leaving client-specific queueing helpers to client wrappers.

**Why.** Exposing mutable setters or ad-hoc registration methods directly on
Context complicates thread safety, alters object configuration mid-lifecycle,
and clutters the core API with client-specific queueing semantics. Passing
configuration through ContextOptions guarantees immutability throughout the
context's lifetime, while encapsulating completion callbacks in parameter
structs like SubmitInfo or InsertRecordingInfo keeps submission metadata and
synchronization cohesive.

## Examples

Bad:

```cpp
// Bad: Exposing mutable setters or standalone callback registration on Context
class SK_API Context final {
public:
    void setPipelineCallback(PipelineCallback callback, PipelineCallbackContext context);
    void insertFinishProc(GpuFinishedProc proc, GpuFinishedContext context);
    bool submit(SubmitInfo submitInfo = {});
};
```

Good:

```cpp
// Good: Configure lifetime callbacks via Options and attach completion procs via SubmitInfo
struct SK_API ContextOptions {
    PipelineCallback fPipelineCallback = nullptr;
    PipelineCallbackContext fPipelineCallbackContext = nullptr;
};

struct SK_API SubmitInfo {
    GpuFinishedProc fFinishedProc = nullptr;
    GpuFinishedContext fFinishedContext = nullptr;
};

class SK_API Context final {
public:
    bool submit(SubmitInfo submitInfo = {});
};
```
