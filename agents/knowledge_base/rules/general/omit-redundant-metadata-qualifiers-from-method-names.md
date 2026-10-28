---
name: skia/omit-redundant-metadata-qualifiers-from-method-names
description: When naming inspection, scanning, or extraction methods, omit redundant qualifiers like 'MetaData' or 'Info' when class context and parameters already describe the operation
rubric: readability
severity: should_fix
---

**Rule.** Do not append redundant or overly verbose qualifiers such as
'MetaData', 'Data', or 'Info' to method names when the enclosing class context,
operation verb, and parameter list already make clear what is being queried or
extracted. Prefer direct, concise verb-noun method names (such as scanFont or
scanFace) over verbose descriptor suffixes.

**Why.** Tacking generic descriptors like 'MetaData' or 'Info' onto inspection
and extraction methods adds syntactic noise without conveying new information.
When the enclosing class, verb, and out-parameters already define the data being
inspected, concise method names keep API declarations readable and consistent.

## Examples

Bad:

```cpp
// Redundant 'MetaData' qualifier when class and parameters make context clear:
virtual bool scanFontMetaData(SkStreamAsset* stream, int ttcIndex,
                              SkString* name, SkFontStyle* style, bool* isFixedPitch,
                              AxisDefinitions* axes) const = 0;
```

Good:

```cpp
// Direct, concise method name relying on parameters and class context:
virtual bool scanFont(SkStreamAsset* stream, int ttcIndex,
                      SkString* name, SkFontStyle* style, bool* isFixedPitch,
                      AxisDefinitions* axes) const = 0;
```
