---
name: skia/do-not-assert-read-buffer-results
description: When deserializing objects or fields from SkReadBuffer, do not assert non-nullness or validity using SkASSERT; handle failures gracefully or via buffer.validate()
rubric: correctness
severity: should_fix
---

**Rule.** Do not use SkASSERT to assert that objects, fields, or resources
deserialized from an SkReadBuffer are non-null or valid. Handle deserialization
failures gracefully with safe fallbacks or record an unflattening error via
buffer.validate().

**Why.** Data read from SkReadBuffer originates from external, potentially
corrupted, truncated, or untrusted streams, or may omit unsupported custom
resources. Deserialization failure is an expected runtime parsing error rather
than an internal invariant violation; using SkASSERT causes debug builds and
fuzzers to crash on malformed input rather than halting unflattening safely.

## Examples

Bad:

```cpp
if (packed & kHas_Typeface_Bit) {
    font->fTypeface = buffer.readTypeface();
    SkASSERT(font->fTypeface);
}
```

Good:

```cpp
if (packed & kHas_Typeface_Bit) {
    font->setTypeface(buffer.readTypeface());
}
// Or when the deserialized resource is mandatory to continue unflattening:
if (!buffer.validate(font->typeface() != nullptr)) {
    return false;
}
```
