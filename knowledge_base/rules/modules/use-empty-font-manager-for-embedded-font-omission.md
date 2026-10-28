---
name: skia/modules/use-empty-font-manager-for-embedded-font-omission
description: When configuring CanvasKit or size-critical builds without embedded fonts, enable skia_enable_fontmgr_custom_empty and select fontmgr_custom_empty_factory instead of dummy font tables
rubric: safety
severity: should_fix
---

**Rule.** When configuring CanvasKit or WebAssembly builds that omit embedded
fonts, enable the dedicated empty font manager
(skia_enable_fontmgr_custom_empty=true) and select its factory target
(skia_fontmgr_factory=":fontmgr_custom_empty_factory"). Do not retain
skia_enable_fontmgr_custom_embedded with dummy or empty font data tables.

**Why.** The embedded font manager (SkFontMgr_custom_embedded) pulls in font
parsing and table-loading code that bloats binary size even when given empty
font arrays. The dedicated custom empty font manager excludes that loading code
entirely, reducing WebAssembly footprint without linker conflicts.

## Examples

Bad:

```cpp
elif [[ $@ == *no_embedded_font* ]]; then
  # Retains custom_embedded with dummy empty font table, wasting code size
  GN_FONT+="skia_enable_fontmgr_custom_embedded=true skia_enable_fontmgr_custom_empty=false"
```

Good:

```cpp
elif [[ $@ == *no_embedded_font* ]]; then
  # Enables custom_empty to omit font parsing and table loading code
  GN_FONT+="skia_enable_fontmgr_custom_embedded=true skia_enable_fontmgr_custom_empty=true "
  GN_FONT+="skia_fontmgr_factory=\":fontmgr_custom_empty_factory\""
```
