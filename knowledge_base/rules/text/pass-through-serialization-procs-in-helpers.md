---
name: skia/text/pass-through-serialization-procs-in-helpers
description: When wrapping serialization or deserialization functions accepting SkSerialProcs or SkDeserialProcs, forward procs explicitly instead of hardcoding empty procs or adding default arguments
rubric: correctness
severity: should_fix
---

**Rule.** When defining wrapper methods or test helpers that invoke
serialization or deserialization routines (such as for slugs, pictures, or text
blobs), accept and forward custom SkSerialProcs or SkDeserialProcs parameters
instead of hardcoding empty procs (`{}`). For internal helpers without
backward-compatibility requirements, require callers to pass the procs
explicitly rather than adding redundant default parameter arguments.

**Why.** Hardcoding empty procs in intermediate wrappers prevents callers and
tests from supplying custom callbacks (such as typeface decoders or image
serializers) necessary to correctly process nested objects. Omitting default
arguments on internal helpers forces callers to deliberately supply intended
procs, avoiding silent fallback to empty defaults.

## Examples

Bad:

```cpp
// Bad: Hardcodes empty procs, preventing custom decoders or typefaces from being supplied
sk_sp<sktext::gpu::Slug> SkStrikeClient::deserializeSlugForTest(const void* data, size_t size) const {
    return sktext::gpu::Slug::Deserialize(data, size, this, {});
}
```

Good:

```cpp
// Good: Forwards procs explicitly without adding unnecessary default arguments
sk_sp<sktext::gpu::Slug> SkStrikeClient::deserializeSlugForTest(const void* data, size_t size,
                                                                const SkDeserialProcs& procs) const {
    return sktext::gpu::Slug::Deserialize(data, size, this, procs);
}
```
