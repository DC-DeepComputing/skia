---
name: skia/api/pass-contextual-metadata-to-deserialization-procs
description: When introducing or updating resource deserialization callback procs, pass all parsed stream metadata such as alpha type so custom procs can replicate default behavior
rubric: correctness
severity: should_fix
---

**Rule.** When defining or introducing callback signatures to delegate resource
deserialization or decoding (such as SkDeserialImageFromDataProc in
SkDeserialProcs), pass all parsed stream metadata and contextual parameters
(such as std::optional<SkAlphaType>) to the callback. Ensure invoking code
forwards these parsed properties alongside raw buffer data.

**Why.** If a deserialization or decoding callback receives only raw data
without parsed stream metadata (such as an explicit alpha type encoded in the
stream), client callbacks cannot accurately replicate or replace default engine
decoding behavior.

## Examples

Bad:

```cpp
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data, void* ctx);
...
if (dProcs.fImageDataProc) {
    image = dProcs.fImageDataProc(data, dProcs.fImageCtx);
}
```

Good:

```cpp
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data,
                                                       std::optional<SkAlphaType> alphaType,
                                                       void* ctx);
...
if (dProcs.fImageDataProc) {
    image = dProcs.fImageDataProc(data, alphaType, dProcs.fImageCtx);
}
```
