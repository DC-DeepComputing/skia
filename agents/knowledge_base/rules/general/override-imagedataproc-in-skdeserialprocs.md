---
name: skia/override-imagedataproc-in-skdeserialprocs
description: When defining or overriding deserialization callbacks in SkDeserialProcs, use sk_sp<SkData> (fImageDataProc) rather than raw buffer pointers (fImageProc)
rubric: safety
severity: should_fix
---

**Rule.** When defining deserialization callbacks or configuring image
deserialization in SkDeserialProcs, accept and use sk_sp<SkData> (such as
fImageDataProc / SkDeserialImageFromDataProc) rather than raw buffer pointers
(const void* data, size_t length) like fImageProc. In SkDeserialProcs, set or
clear fImageDataProc instead of setting only fImageProc, which is shadowed by
default procs and forces unsafe non-owning wrappers or redundant copies.

**Why.** Accepting sk_sp<SkData> enables persistent objects (such as SkImage via
SkImages::DeferredFromEncodedData) to safely retain shared ownership with zero
copies. Raw pointer callbacks force callers to either perform redundant deep
copies or wrap pointers via SkData::MakeWithoutCopy, risking use-after-free
crashes if the object outlives the read buffer. Furthermore, SkReadBuffer
prioritizes fImageDataProc over fImageProc, silently ignoring fImageProc when
fImageDataProc is set.

## Examples

Bad:

```cpp
// Bad: Raw pointer callback forces unsafe MakeWithoutCopy or deep copies;
// setting only fImageProc is also silently shadowed by fImageDataProc.
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data, size_t length,
                                               std::optional<SkAlphaType>, void* ctx);

SkDeserialProcs procs = ToolUtils::default_deserial_procs();
procs.fImageProc = [](const void* data, size_t size, std::optional<SkAlphaType> at, void* ctx) {
    sk_sp<SkData> tmpData = SkData::MakeWithoutCopy(data, size);
    return SkImages::DeferredFromEncodedData(std::move(tmpData));
};
```

Good:

```cpp
// Good: Pass sk_sp<SkData> for safe zero-copy shared ownership,
// and override fImageDataProc directly in SkDeserialProcs.
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data,
                                                       std::optional<SkAlphaType>, void* ctx);

SkDeserialProcs procs = ToolUtils::default_deserial_procs();
procs.fImageDataProc = [](sk_sp<SkData> data, std::optional<SkAlphaType> at, void* ctx) {
    return SkImages::DeferredFromEncodedData(std::move(data));
};
```
