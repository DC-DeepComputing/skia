---
name: skia/gpu/avoid-noop-gpu-work-submissions
description: When implementing or enqueuing GpuWorkSubmission objects in Graphite's QueueManager, do not use dummy or no-op waitUntilFinished implementations that break FIFO synchronization
rubric: correctness
severity: should_fix
---

**Rule.** Do not enqueue dummy, marker, or placeholder GpuWorkSubmission objects
with no-op waitUntilFinished() or onWaitUntilFinished() methods into
QueueManager's outstanding submissions. Every submitted work object must
genuinely synchronize its command buffer on the GPU when waited upon.

**Why.** QueueManager::checkForFinishedWork(SyncToCpu::kYes) relies on GPU
command streams completing in FIFO order and synchronizes the CPU solely by
calling waitUntilFinished() on the most recent submission
(fOutstandingSubmissions.back()). If a marker submission with a no-op wait is at
the back of the queue, the CPU sync returns immediately while prior GPU work
remains active, leading to data races and assertion failures.

## Examples

Bad:

```cpp
// Bad: Enqueuing a dummy submission that returns immediately without waiting for GPU work
class DummyWorkSubmission final : public GpuWorkSubmission {
    bool onIsFinished(const SharedContext*) override { return true; }
    void onWaitUntilFinished(const SharedContext*) override {} // No-op wait
};
queueManager->onSubmitToGpu(std::make_unique<DummyWorkSubmission>(...));
```

Good:

```cpp
// Good: Every submission must genuinely block until its GPU work completes
class BackendWorkSubmission final : public GpuWorkSubmission {
    bool onIsFinished(const SharedContext* ctx) override {
        return fFence->isSignaled();
    }
    void onWaitUntilFinished(const SharedContext* ctx) override {
        fFence->wait();
    }
};
```
