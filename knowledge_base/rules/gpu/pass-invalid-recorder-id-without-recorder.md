---
name: skia/gpu/pass-invalid-recorder-id-without-recorder
description: When instantiating a Graphite ResourceProvider not attached to a Recorder, pass SK_InvalidGenID as the recorderID rather than generating synthetic IDs
rubric: correctness
severity: should_fix
---

**Rule.** When creating or instantiating a Graphite ResourceProvider that is not
associated with a Recorder (such as for precompilation or Context
initialization), pass SK_InvalidGenID as the recorderID parameter. Do not
generate synthetic or arbitrary IDs from local counters to satisfy the
parameter.

**Why.** Graphite's ResourceCache uses the recorderID parameter exclusively to
create message-passing mailboxes in ProxyCache for genuine Recorders. Generating
synthetic IDs from a separate counter risks colliding with sequentially
allocated Recorder IDs, whereas passing SK_InvalidGenID prevents ID conflation
and avoids creating an unneeded ProxyCache.

## Examples

Bad:

```cpp
// Bad: Generating synthetic IDs risks colliding with real Recorder IDs in ProxyCache.
uint32_t next_resource_provider_id() {
    static std::atomic<uint32_t> nextID{1};
    return nextID.fetch_add(1, std::memory_order_relaxed);
}
...
auto provider = sharedContext->makeResourceProvider(&singleOwner,
                                                    next_resource_provider_id(),
                                                    kEmptyBudget);
```

Good:

```cpp
// Good: Pass SK_InvalidGenID when creating a ResourceProvider without a Recorder.
auto provider = sharedContext->makeResourceProvider(&singleOwner,
                                                    SK_InvalidGenID,
                                                    kEmptyBudget);
```
