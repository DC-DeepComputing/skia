---
name: skia/gpu/fallback-generic-key-serialization
description: When pretty-printing or formatting structured GPU keys with specialized decoders, fall back to generic data serialization instead of emitting an invalid indicator
rubric: correctness
severity: should_fix
---

**Rule.** When pretty-printing, dumping, or formatting structured key data with
specialized decoders, fall back to generic data serialization (such as base64 or
raw hex) rather than emitting an '(invalid)' error indicator when specialized
decoding fails or is inapplicable. Never treat unrecognized or non-descriptive
payload configurations as corrupt data.

**Why.** Specialized decoders (such as human-readable sampler or conversion
descriptors) only recognize a subset of valid payloads. Emitting an '(invalid)'
error string when specialized decoding is inapplicable falsely implies the key
is corrupt and prevents debugging tools from inspecting the raw payload data.

## Examples

Bad:

```cpp
if (info.isValid()) {
    str->append(caps->toString(info));
} else {
    str->append("(invalid)");
}
```

Good:

```cpp
bool descriptiveFormAppended = false;
if (s.isImmutable()) {
    std::string tmp = caps->toString(s.immutableSamplerInfo());
    if (!tmp.empty()) {
        descriptiveFormAppended = true;
        str->append(tmp);
    }
}
if (!descriptiveFormAppended) {
    append_as_base64(str, { &keyData[currentIndex], dataIndexCount });
}
```
