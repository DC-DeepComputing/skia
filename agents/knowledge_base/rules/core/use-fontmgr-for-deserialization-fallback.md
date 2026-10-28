---
name: skia/core/use-fontmgr-for-deserialization-fallback
description: When resolving typefaces during deserialization with an optional fallback SkFontMgr, query that manager's legacyMakeTypeface and return an empty typeface if null
rubric: correctness
severity: should_fix
---

**Rule.** When deserializing typefaces or resolving fallback fonts with an
explicit fallback or last-resort font manager (SkFontMgr), resolve font families
using that manager (e.g. fontMgr->legacyMakeTypeface(...)) rather than global
helpers. If no font manager is provided, return an empty typeface
(SkTypeface::MakeEmpty()) rather than falling back to global singletons.

**Why.** Global font creation helpers implicitly rely on default font
singletons, bypassing caller-supplied fallback font managers and violating
dependency injection. When an explicit font manager is provided, querying it
directly respects caller configuration, and returning an empty typeface when
omitted avoids unmanaged global state.

## Examples

Bad:

```cpp
// Bypasses caller-provided lastResortMgr and relies on global defaults
return SkTypeface::MakeFromName(desc.getFamilyName(), desc.getStyle());
```

Good:

```cpp
if (lastResortMgr) {
    return lastResortMgr->legacyMakeTypeface(desc.getFamilyName(), desc.getStyle());
}
return SkTypeface::MakeEmpty();
```
