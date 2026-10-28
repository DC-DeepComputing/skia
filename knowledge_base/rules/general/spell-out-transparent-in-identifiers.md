---
name: skia/spell-out-transparent-in-identifiers
description: When naming identifiers representing transparent colors, shaders, or paints, spell out 'transparent' fully rather than using ambiguous contractions like 'xparent'
rubric: readability
severity: nit
---

**Rule.** When naming variables, functions, or parameters representing
transparent colors, paints, shaders, or transparency states, spell out
'transparent' (or 'transparency') fully. Do not use informal or non-standard
contractions such as 'xparent' or 'xparency'.

**Why.** Informal contractions like 'xparent' are ambiguous and easily misread
as 'ex-parent' or 'cross-parent'. Spelling out 'transparent' maintains
readability and consistency across the codebase.

## Examples

Bad:

```cpp
PaintOptions xparent_paint_image_premul_hw_and_clamp_srcover();
SkColor xparent_color = SK_ColorTRANSPARENT;
```

Good:

```cpp
PaintOptions transparent_paint_image_premul_hw_and_clamp_srcover();
SkColor transparent_color = SK_ColorTRANSPARENT;
```
