---
name: skia/avoid-makewithoutcopy-on-transient-data
description: When wrapping buffers with SkData::MakeWithoutCopy, avoid passing to retaining factories or decoders unless memory is static or guaranteed to outlive consumers
rubric: safety
severity: should_fix
---

**Rule.** Do not use SkData::MakeWithoutCopy to wrap transient, stack-allocated,
or unowned buffers passed to factory functions, decoders, or consumers that may
retain or defer decoding from the SkData; use SkData::MakeWithCopy instead.
Restrict MakeWithoutCopy to static or process-lifetime memory, or strictly
bounded synchronous scopes where the caller guarantees the underlying buffer
outlives all consumers, and document the lifetime invariant in an inline
comment.

**Why.** SkData::MakeWithoutCopy wraps existing memory with a no-op release proc
without extending buffer lifetime. Passing it to decoders or retaining factories
(such as SkImages::DeferredFromEncodedData) risks use-after-free crashes if the
underlying buffer is deallocated or overwritten while the object continues to
reference it.

## Examples

Bad:

```cpp
// Transient buffer passed to a deferred or retaining factory
sk_sp<SkData> data = SkData::MakeWithoutCopy(ptr, size);
return SkImages::DeferredFromEncodedData(std::move(data));
```

Good:

```cpp
// Retained or deferred consumers require an owning copy
sk_sp<SkData> data = SkData::MakeWithCopy(ptr, size);
return SkImages::DeferredFromEncodedData(std::move(data));

// Ephemeral synchronous use is permitted if bounded and documented
// Safe: 'mip' is read and discarded locally, so buffer outlives image.
sk_sp<SkImage> mip = deserialize_image(SkData::MakeWithoutCopy(ptr, size), dProcs, alphaType);
```
