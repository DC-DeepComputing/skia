---
name: skia/modules/do-not-wrap-skfourbytetag
description: When passing or exposing four-byte OpenType or script tags in SkShaper consumers, use SkFourByteTag directly rather than creating custom wrapper abstractions
rubric: design
severity: nit
---

**Rule.** Do not create custom wrapper abstractions, enums, or helper layers to
hide SkFourByteTag or SkSetFourByteTag in SkShaper callers and text utilities.
Use SkFourByteTag and SkSetFourByteTag directly as the canonical representation
for OpenType and ISO 15924 script tags across Skia's shaping interfaces.

**Why.** SkFourByteTag is Skia's standard cross-platform representation for
four-character OpenType feature and script tags across SkShaper and font
interfaces. Hiding it behind intermediate types or custom abstractions adds
unnecessary indirection and maintenance complexity without functional benefit.

## Examples

Bad:

```cpp
// Introducing an intermediate wrapper or questioning direct SkFourByteTag usage
enum class ScriptTag { Latn, Unknown };
SkFourByteTag toTag(ScriptTag t);

std::unique_ptr<SkShaper::ScriptRunIterator> script =
        SkShaper::MakeScriptRunIterator(utf8Text, textByteLen, toTag(ScriptTag::Unknown));
```

Good:

```cpp
// Use SkFourByteTag and SkSetFourByteTag directly with SkShaper APIs
std::unique_ptr<SkShaper::ScriptRunIterator> script =
        SkShaper::MakeScriptRunIterator(utf8Text, textByteLen, SkSetFourByteTag('Z','z','z','z'));
```
