---
name: skia/validate-sentinel-on-deserialization-fallback
description: When deserializing from SkReadBuffer where a field encodes a known identifier or fallback payload, validate that unrecognized values match the expected sentinel via buffer.validate()
rubric: safety
severity: should_fix
---

**Rule.** When deserializing untrusted data from SkReadBuffer where a field
distinguishes between a recognized key/identifier and an inline fallback
representation, validate that any unrecognized value equals the expected
sentinel (e.g., buffer.validate(key == 0)). Do not silently take the fallback
branch on unrecognized non-sentinel values without validating and marking the
buffer invalid.

**Why.** Data from SkReadBuffer originates from external, untrusted streams. If
an unrecognized non-sentinel value is silently interpreted as the fallback
branch, corrupted or malicious streams bypass validation, misparse subsequent
fields, and introduce deserialization vulnerabilities. Calling buffer.validate()
ensures unknown values mark the buffer invalid and halt deserialization safely.

## Examples

Bad:

```cpp
uint32_t candidateStableKey = buffer.readUInt();
auto effect = SkKnownRuntimeEffects::MaybeGetKnownRuntimeEffect(candidateStableKey);
if (!effect) {
    // Silently assumes candidateStableKey == 0 without validation
    SkString sksl;
    buffer.readString(&sksl);
    effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
}
```

Good:

```cpp
uint32_t candidateStableKey = buffer.readUInt();
auto effect = SkKnownRuntimeEffects::MaybeGetKnownRuntimeEffect(candidateStableKey);
if (!effect && !buffer.validate(candidateStableKey == 0)) {
    return nullptr;
}
if (!effect) {
    SkString sksl;
    buffer.readString(&sksl);
    effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
}
```
