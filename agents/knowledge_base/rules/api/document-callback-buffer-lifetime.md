---
name: skia/api/document-callback-buffer-lifetime
description: When documenting callbacks or procs accepting non-owning buffer arguments, specify that copying is only required if retained after invocation
rubric: readability
severity: nit
---

**Rule.** When documenting callbacks, function pointers, or proc types that
accept non-owning buffer arguments (such as `const void* data, size_t length` or
`SkSpan`), specify that copying the buffer is only required if the data is
retained after the callback invocation finishes. Do not imply that copying is
unconditionally mandatory when immediate, synchronous processing does not
require a copy.

**Why.** Non-owning buffer parameters passed to callbacks remain valid for the
duration of the call. Phrasing comments as an unconditional copy requirement
misleads callers into performing redundant defensive copies during synchronous,
greedy processing or decoding, incurring unnecessary heap allocations and
performance overhead.

## Examples

Bad:

```cpp
/**
 * Implementations of SkDeserialImageProc must make a copy of any data
 * it needs, since the buffer will go away after serialization ends.
 */
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data,
                                               size_t length,
                                               void* ctx);
```

Good:

```cpp
/**
 * Implementations of SkDeserialImageProc must make a copy of any data
 * they need after the proc finishes, since the buffer will go away
 * after serialization ends. Synchronous decoding requires no copy.
 */
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data,
                                               size_t length,
                                               void* ctx);
```
