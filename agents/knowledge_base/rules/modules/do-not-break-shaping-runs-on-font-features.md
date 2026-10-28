---
name: skia/modules/do-not-break-shaping-runs-on-font-features
description: When segmenting text into shaping runs or comparing font styles, do not split runs on font feature differences; apply features across unified runs
rubric: correctness
severity: should_fix
---

**Rule.** Do not split text into separate shaping runs or font blocks based on
differences in font features. Treat OpenType features as per-codepoint or
per-range attributes within a single shaping run rather than run-breaking font
style attributes.

**Why.** In OpenType and HarfBuzz, font features (such as ligatures, small caps,
or stylistic alternates) are range-based attributes attached to codepoints and
passed directly to the shaper. Breaking runs at font feature boundaries disrupts
contextual shaping, ligatures, and kerning between adjacent characters.

## Examples

Bad:

```cpp
// Bad: font features are treated as run-breaking font attributes
case kFont:
    return fFontStyle == other.fFontStyle &&
           fLocale == other.fLocale &&
           fFontFamilies == other.fFontFamilies &&
           fFontSize == other.fFontSize &&
           fFontFeatures == other.fFontFeatures;
```

Good:

```cpp
// Good: font features are passed as range attributes to the shaper, not run delimiters
case kFont:
    return fFontStyle == other.fFontStyle &&
           fLocale == other.fLocale &&
           fFontFamilies == other.fFontFamilies &&
           fFontSize == other.fFontSize;
```
