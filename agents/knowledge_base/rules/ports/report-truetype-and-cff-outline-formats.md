---
name: skia/ports/report-truetype-and-cff-outline-formats
description: When implementing SkTypeface::onGetAdvancedMetrics, identify and report both TrueType and CFF outline formats in SkAdvancedTypefaceMetrics instead of omitting CFF
rubric: correctness
severity: should_fix
---

**Rule.** In SkTypeface::onGetAdvancedMetrics implementations, detect and assign
both kTrueType_Font and kCFF_Font outline formats to
SkAdvancedTypefaceMetrics::fType rather than checking only for TrueType
outlines. Ensure font backends populate both formats consistently even if
downstream consumers fall back on Type3 for CFF.

**Why.** All SkTypeface backends (FreeType, CoreText, DirectWrite, Fontations)
should report consistent and complete font metadata. Omitting CFF outline
detection leaves backend capabilities incomplete and prevents downstream
consumers from taking advantage of CFF font data.

## Examples

Bad:

```cpp
if (format == OutlineFormat::Glyf) {
    info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
}
```

Good:

```cpp
switch (format) {
    case OutlineFormat::Glyf:
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
        break;
    case OutlineFormat::Cff:
        info->fType = SkAdvancedTypefaceMetrics::kCFF_Font;
        break;
    default:
        break;
}
```
