---
name: skia/document-why-default-serialization-procs-are-safe
description: When instantiating SkReadBuffer or SkWriteBuffer with empty or default SkDeserialProcs or SkSerialProcs, document why custom procs are not required
rubric: readability
severity: nit
---

**Rule.** When instantiating an SkReadBuffer or SkWriteBuffer with empty or
default SkDeserialProcs or SkSerialProcs, add an inline comment explaining why
custom procs are not needed for that payload. Specifically document that the
serialized data is guaranteed not to contain resources such as SkImages or
custom typefaces, or why default handling is safe.

**Why.** Omitting SkDeserialProcs or SkSerialProcs causes image and resource
decoding or encoding to be bypassed or to fail silently. Leaving empty or
default procs unannotated creates ambiguity over whether missing procs are an
accidental omission that risks runtime failures, or whether the payload format
is intentionally restricted to non-image data.

## Examples

Bad:

```cpp
// Bad: Ambiguous whether SkImages can be present or if deserialization procs were omitted by mistake.
SkReadBuffer buffer{safeMemory->data(), safeMemory->size()};
```

Good:

```cpp
// Good: Explains the payload invariant guaranteeing no image deserialization is needed.
// We do not need to set any SkDeserialProcs here because SkStrikeServerImpl::writeStrikeData
// did not encode any SkImages.
SkReadBuffer buffer{safeMemory->data(), safeMemory->size()};
```
