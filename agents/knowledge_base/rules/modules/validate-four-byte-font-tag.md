---
name: skia/modules/validate-four-byte-font-tag
description: When converting string inputs into 4-byte OpenType font tags via SkSetFourByteTag, validate 4-byte length and printable ASCII range [0x20, 0x7E]
rubric: safety
severity: should_fix
---

**Rule.** When converting string inputs into 4-byte OpenType font tags (such as
font variation axes or feature tags via SkSetFourByteTag), validate that the
string length is exactly 4 bytes and that all characters fall within the
printable ASCII range [0x20, 0x7E] before indexing. Return std::nullopt or
reject invalid strings to avoid out-of-bounds reads and spec violations.

**Why.** Directly indexing string characters (e.g. s[0], s[1], s[2], s[3])
without validating size causes out-of-bounds memory reads on inputs shorter than
4 bytes. Additionally, OpenType specifications and CSS font-variation-settings
require font tags to consist strictly of four printable ASCII characters in the
range 0x20 to 0x7E.

## Examples

Bad:

```cpp
const std::string_view tag = jaxis.fKey.str();
var.push_back({
    .axis  = SkSetFourByteTag(tag[0], tag[1], tag[2], tag[3]),
    .value = static_cast<float>(**jval),
});
```

Good:

```cpp
const auto make_tag = [](const std::string_view& s) -> std::optional<SkFourByteTag> {
    // https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/font-variation-settings#string_number
    if (s.size() != 4 || !std::all_of(s.cbegin(), s.cend(),
                                      [](char c) { return c >= 0x20 && c <= 0x7e; })) {
        return std::nullopt;
    }
    return SkSetFourByteTag(s[0], s[1], s[2], s[3]);
};
```
