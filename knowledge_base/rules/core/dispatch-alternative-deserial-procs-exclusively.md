---
name: skia/core/dispatch-alternative-deserial-procs-exclusively
description: When dispatching alternative deserialization callback variants (such as SkDeserialProcs fImageDataProc and fImageProc), treat them as mutually exclusive rather than falling through on failure
rubric: safety
severity: should_fix
---

**Rule.** When dispatching to alternative client callback variants for the same
operation (such as SkDeserialProcs::fImageDataProc and fImageProc), treat
registered callbacks as mutually exclusive alternatives using 'else if'
branches. Do not fall through to invoke the secondary or legacy callback when
the primary callback returns null or fails.

**Why.** If a client registers both modern and legacy callback variants for
compatibility across Skia versions, falling through to the legacy proc when the
primary proc returns null causes invalid or corrupted data to be inspected and
rejected twice. Furthermore, legacy procs that accept raw pointers and buffer
sizes often redundantly copy memory before failing.

## Examples

Bad:

```cpp
if (dProcs.fImageDataProc) {
    image = dProcs.fImageDataProc(data, alphaType, dProcs.fImageCtx);
}
// Anti-pattern: falls through to legacy callback on failure, repeating work and copying buffers
if (!image && dProcs.fImageProc) {
    image = dProcs.fImageProc(data->data(), data->size(), alphaType, dProcs.fImageCtx);
}
```

Good:

```cpp
// Mutually exclusive: execute preferred proc if present, otherwise fall back to legacy proc
if (dProcs.fImageDataProc) {
    image = dProcs.fImageDataProc(data, alphaType, dProcs.fImageCtx);
} else if (dProcs.fImageProc) {
    image = dProcs.fImageProc(data->data(), data->size(), alphaType, dProcs.fImageCtx);
}
```
