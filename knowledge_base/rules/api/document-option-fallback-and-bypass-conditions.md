---
name: skia/api/document-option-fallback-and-bypass-conditions
description: When documenting public options, callbacks, or proc hooks, specify invocation conditions, precedence over deprecated members, and baseline requirements for opportunistic optimizations
rubric: readability
severity: should_fix
---

**Rule.** When documenting configuration options, callbacks, or proc structs in
public headers (such as ContextOptions or SkDeserialProcs), specify exact
conditions under which an entry is ignored, bypassed, or invoked, avoiding vague
phrasing like 'might be a no-op'. When deprecating a setting or callback in
favor of a replacement, document precedence and invocation conditions.
Additionally, when an opportunistic or optimized callback exists alongside a
baseline fallback, do not instruct clients to set only the optimized variant;
clearly document whether the baseline callback remains required for fallback
paths.

**Why.** Vague comments stating that a setting 'might be a no-op' or omitting
precedence rules leave API consumers uncertain when an option takes effect or
why setting it produces no behavior change. Similarly, advising callers to
implement only an opportunistic optimization when internal code paths still
require the baseline fallback causes runtime failures or missed data decoding
when fallback paths are taken. Explicitly documenting bypass conditions,
precedence, and baseline requirements ensures clients implement robust
configurations.

## Examples

Bad:

```cpp
// Bad: vague phrasing leaves clients uncertain when the option is active.
/**
 * Note: this option might be a no-op if the backend doesn't implement it.
 */
std::optional<SkISize> fInternalMSAATileSize = std::nullopt;

// Bad: omits precedence and exact invocation conditions for deprecated callback.
/**
 * Deprecated version of the Pipeline callback. It corresponds to the kAddingPipeline
 * case of PipelineCachingCallback but only invokes if serializable.
 */
using PipelineCallback = void (*)(PipelineCallbackContext context, sk_sp<SkData> data);

// Bad: tells clients to set only the optimized proc when baseline fallback is needed.
/**
 * Clients should only set one of these, and preferably the SkDeserialImageFromDataProc
 * version to avoid an extra copy of the data.
 */
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data, size_t length, void* ctx);
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data, void* ctx);
```

Good:

```cpp
// Good: explicitly documents backend support and hardware bypass conditions.
/**
 * Note: this option will be ignored if the backend doesn't support it, or if a more
 * optimal HW feature is available.
 */
std::optional<SkISize> fInternalMSAATileSize = std::nullopt;

// Good: explicitly documents precedence and exact invocation conditions.
/**
 * Deprecated version of the Pipeline callback. This callback is only invoked for
 * PipelineCacheOp::kAddingPipeline ops and when serializable. It is ignored if
 * fPipelineCachingCallback is set.
 */
using PipelineCallback = void (*)(PipelineCallbackContext context, sk_sp<SkData> data);

// Good: clearly states baseline requirement and opportunistic invocation conditions.
/**
 * Clients should set at least SkDeserialImageProc; SkDeserialImageFromDataProc may be called
 * if the internal implementation has an SkData copy already.
 */
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data, size_t length, void* ctx);
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData>, void* ctx);
```
