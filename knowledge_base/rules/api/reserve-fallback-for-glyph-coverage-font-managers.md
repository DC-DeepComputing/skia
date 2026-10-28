---
name: skia/api/reserve-fallback-for-glyph-coverage-font-managers
description: When naming SkFontMgr parameters or font providers, reserve 'fallback' for glyph coverage and use 'lastResort' for emergency deserialization or lookup
rubric: readability
severity: should_fix
---

**Rule.** In font and text APIs, do not use 'fallback' (such as fallbackMgr) to
name an SkFontMgr or font provider unless it specifically performs font fallback
for character or glyph coverage. Use names like 'lastResortMgr' for emergency,
default, or last-ditch font lookups and deserialization.

**Why.** In typography and Skia text layout, 'fallback' specifically denotes
finding and resolving typefaces to provide glyph coverage when a font lacks
requested characters. Using 'fallback' for an emergency font manager or
deserializer conflates glyph-level fallback with last-resort object lookup,
creating confusion about whether the manager performs font fallback shaping.

## Examples

Bad:

```cpp
// Misleading: implies character/glyph coverage fallback
sk_sp<SkTypeface> MakeDeserialize(SkStream* stream,
                                  sk_sp<SkFontMgr> fallbackMgr,
                                  const SkFontArguments* args = nullptr);
```

Good:

```cpp
// Clear: clarifies this manager is used only as an emergency lookup
sk_sp<SkTypeface> MakeDeserialize(SkStream* stream,
                                  sk_sp<SkFontMgr> lastResortMgr,
                                  const SkFontArguments* args = nullptr);
```
