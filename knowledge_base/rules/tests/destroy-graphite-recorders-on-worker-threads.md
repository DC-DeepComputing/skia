---
name: skia/tests/destroy-graphite-recorders-on-worker-threads
description: When recording drawing commands on worker threads in Graphite, transfer Recorder ownership via std::unique_ptr rather than retaining and destroying on the Context thread
rubric: safety
severity: should_fix
---

**Rule.** Do not retain skgpu::graphite::Recorder or PrecompileContext instances
on the Context-owning thread under the assumption that they require destruction
on that same thread. Transfer ownership directly to worker threads via
std::unique_ptr using std::move, allowing the worker thread to own and destroy
the recorder upon completion.

**Why.** Graphite's multi-threaded recording architecture is designed without
thread affinity for Recorder or PrecompileContext instances; their lifetime and
destruction are fully thread-safe across worker threads. Forcing the
Context-owning thread to retain references and synchronize manual destruction
complicates thread management without providing any safety benefit.

## Examples

Bad:

```cpp
// Unnecessarily keeping Recorder ownership on the Context-owning thread
std::unique_ptr<Recorder> recorders[kNumThreads];
for (int i = 0; i < kNumThreads; ++i) {
    recorders[i] = context->makeRecorder();
    threads.push_back(std::thread(record_work, recorders[i].get()));
}
for (auto& thread : threads) {
    thread.join();
}
for (auto& recorder : recorders) {
    recorder.reset();
}
```

Good:

```cpp
// Move ownership directly to worker threads
for (int i = 0; i < kNumThreads; ++i) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    threads.push_back(std::thread(record_work, std::move(recorder)));
}
for (auto& thread : threads) {
    thread.join();
}
```
