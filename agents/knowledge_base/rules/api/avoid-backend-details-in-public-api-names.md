---
name: skia/api/avoid-backend-details-in-public-api-names
description: When declaring public API methods or client language bindings, name methods after abstract types or operations rather than embedding backend library names like FreeType
rubric: readability
severity: should_fix
---

**Rule.** Do not embed backend or third-party library names or abbreviations
(such as FreeType, Free, HarfBuzz, or ICU) into public API method names or
language bindings. Name public operations after the abstract concept, type, or
action being performed (e.g., MakeTypefaceFromData rather than
MakeFreeTypefaceFromData), keeping public surfaces backend-agnostic.

**Why.** Underlying third-party libraries and backends are internal
implementation details that callers should not depend on and that may be swapped
or deprecated over time. Baking library names or contractions into public method
names couples client contracts to specific backends and creates nonsensical or
inconsistent naming when alternatives are introduced.

## Examples

Bad:

```cpp
// In public language bindings (e.g., CanvasKit):
CanvasKit.Typeface.MakeFreeTypefaceFromData = function(fontData) { ... };

// In public C++ API headers:
static sk_sp<SkTypeface> MakeFreeTypefaceFromData(sk_sp<SkData> data);
```

Good:

```cpp
// In public language bindings (e.g., CanvasKit):
CanvasKit.Typeface.MakeTypefaceFromData = function(fontData) { ... };

// In public C++ API headers:
static sk_sp<SkTypeface> MakeTypefaceFromData(sk_sp<SkData> data);
```
