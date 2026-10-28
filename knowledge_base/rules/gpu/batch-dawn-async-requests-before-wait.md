---
name: skia/gpu/batch-dawn-async-requests-before-wait
description: When issuing multiple Dawn asynchronous operations or popping error scopes, initiate all requests upfront to obtain futures before waiting with WaitAny
rubric: safety
severity: should_fix
---

**Rule.** When popping multiple Dawn error scopes or issuing a batch of
asynchronous WebGPU operations, initiate all requests upfront to obtain their
futures before waiting on them with WaitAny, rather than interleaving each
request with an immediate wait.

**Why.** In Dawn client-server architectures (such as WebGPU wire, IPC, or
Emscripten), interleaving asynchronous requests with synchronous waits incurs
serialized round-trip latency for each call. Initiating all asynchronous
requests upfront pipelines and batches commands over the wire. Using
wgpu::CallbackMode::WaitAnyOnly guarantees callbacks execute only during
WaitAny, ensuring shared mutable callback state remains safe across pipelined
requests.

## Examples

Bad:

```cpp
// Bad: Interleaving async calls and waits forces serialized wire round-trips.
status = instance.WaitAny(fSharedContext->device().PopErrorScope(
                                  wgpu::CallbackMode::WaitAnyOnly, errorCallback, &errorState),
                          /*timeout=*/std::numeric_limits<uint64_t>::max());
...
status = instance.WaitAny(fSharedContext->device().PopErrorScope(
                                  wgpu::CallbackMode::WaitAnyOnly, errorCallback, &errorState),
                          /*timeout=*/std::numeric_limits<uint64_t>::max());
```

Good:

```cpp
// Good: Initiate all async requests upfront to pipeline over the wire, then wait.
auto internalFuture = fSharedContext->device().PopErrorScope(
        wgpu::CallbackMode::WaitAnyOnly, errorCallback, &errorState);
auto oomFuture = fSharedContext->device().PopErrorScope(
        wgpu::CallbackMode::WaitAnyOnly, errorCallback, &errorState);
auto validationFuture = fSharedContext->device().PopErrorScope(
        wgpu::CallbackMode::WaitAnyOnly, errorCallback, &errorState);

status = instance.WaitAny(internalFuture, /*timeout=*/std::numeric_limits<uint64_t>::max());
status = instance.WaitAny(oomFuture, /*timeout=*/std::numeric_limits<uint64_t>::max());
status = instance.WaitAny(validationFuture, /*timeout=*/std::numeric_limits<uint64_t>::max());
```
