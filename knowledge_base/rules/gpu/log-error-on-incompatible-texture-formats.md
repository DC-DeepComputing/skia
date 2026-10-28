---
name: skia/gpu/log-error-on-incompatible-texture-formats
description: When rejecting GPU task or copy operations due to incompatible texture formats, log an error diagnostic with format names rather than silently returning nullptr
rubric: testing
severity: should_fix
---

**Rule.** When validating texture or surface compatibility in GPU tasks, copy
routines, or resource initialization, emit a descriptive error log (such as
SKIA_LOG_E or SKGPU_LOG_E) reporting both mismatched format names before
returning null or failing. Do not silently return nullptr without diagnostic
logging when an operation cannot proceed due to incompatible resource formats.

**Why.** Silent bailouts during GPU task creation or resource copies leave
runtime failures completely untraceable, making it difficult for callers and
developers to diagnose why an operation was rejected. Emitting an error log with
the concrete source and destination format names provides immediate visibility
into the underlying configuration incompatibility.

## Examples

Bad:

```cpp
// Bad: Silently bailing out on format mismatch leaves task creation failure untraceable
if (srcProxy->format() != dstProxy->format()) {
    return nullptr;
}
```

Good:

```cpp
// Good: Log mismatched format names before returning nullptr
TextureFormat srcFormat = srcProxy->format();
TextureFormat dstFormat = dstProxy->format();
if (srcFormat != dstFormat) {
    SKIA_LOG_E("Unable to copy between textures of different formats, src = %s, dst = %s",
               TextureFormatName(srcFormat), TextureFormatName(dstFormat));
    return nullptr;
}
```
