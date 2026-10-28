---
name: skia/ports/omit-postscript-name-in-ongetadvancedmetrics
description: When implementing SkTypeface::onGetAdvancedMetrics in font backends, omit populating fPostScriptName because the base class getAdvancedMetrics wrapper automatically resolves it
rubric: design
severity: should_fix
---

**Rule.** In SkTypeface::onGetAdvancedMetrics() overrides, do not manually query
or assign fPostScriptName in the returned SkAdvancedTypefaceMetrics. Allow the
base class SkTypeface::getAdvancedMetrics() wrapper to automatically populate
fPostScriptName via getPostScriptName() and fallback to getFamilyName().

**Why.** The public SkTypeface::getAdvancedMetrics() Non-Virtual Interface (NVI)
wrapper already checks if fPostScriptName is empty and automatically resolves it
using getPostScriptName() or getFamilyName(). Populating it manually inside
backend overrides duplicates logic and introduces redundant platform queries.

## Examples

Bad:

```cpp
std::unique_ptr<SkAdvancedTypefaceMetrics> Typeface::onGetAdvancedMetrics() const {
    auto info = std::make_unique<SkAdvancedTypefaceMetrics>();
    if (!this->onGetPostScriptName(&info->fPostScriptName)) {
        SkDEBUGF("Unable to get postscript name\n");
    }
    return info;
}
```

Good:

```cpp
std::unique_ptr<SkAdvancedTypefaceMetrics> Typeface::onGetAdvancedMetrics() const {
    auto info = std::make_unique<SkAdvancedTypefaceMetrics>();
    // Leave info->fPostScriptName empty; SkTypeface::getAdvancedMetrics()
    // automatically populates it via getPostScriptName() or getFamilyName().
    return info;
}
```
