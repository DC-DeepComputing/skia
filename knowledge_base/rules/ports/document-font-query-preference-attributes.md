---
name: skia/ports/document-font-query-preference-attributes
description: When adding non-obvious preference attributes to font query patterns like FontConfig, document why the attribute is requested to prioritize valid fonts
rubric: readability
severity: should_fix
---

**Rule.** When adding non-obvious preference attributes to font query patterns
(such as FontConfig's FC_FONT_WRAPPER), document in an inline comment why the
attribute is requested and clarify that it prioritizes valid font types during
matching to avoid subsequent post-match rejection.

**Why.** Many font matching systems (like FontConfig) use weighted scoring
rather than hard filtering for query attributes. Without documentation,
maintainers cannot tell whether a query attribute is a strict requirement or a
soft prioritization hint designed to favor instantiable font containers (like
SFNT) over unsupported formats.

## Examples

Bad:

```cpp
FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
FcPatternAddString(pattern, FC_FONT_WRAPPER, reinterpret_cast<const FcChar8*>("SFNT"));
```

Good:

```cpp
FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
// Prefer SFNT fonts in order to reduce the chance of having to reject the font
// in isValidPattern().
FcPatternAddString(pattern, FC_FONT_WRAPPER, reinterpret_cast<const FcChar8*>("SFNT"));
```
