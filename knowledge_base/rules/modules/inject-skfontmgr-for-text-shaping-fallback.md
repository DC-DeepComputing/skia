---
name: skia/modules/inject-skfontmgr-for-text-shaping-fallback
description: When configuring SkShaper or MakeFontMgrRunIterator for text shaping, do not pass SkFontMgr::RefEmpty() or nullptr; accept an sk_sp<SkFontMgr> to enable font fallback
rubric: correctness
severity: blocking
---

**Rule.** When integrating or configuring text shaping components (such as
SkShaper or SkShaper::MakeFontMgrRunIterator), do not pass SkFontMgr::RefEmpty()
or nullptr if character font fallback is needed. Instead, accept an
sk_sp<SkFontMgr> parameter from the calling application or environment to allow
proper fallback font resolution.

**Why.** In HarfBuzz-backed SkShaper implementations, passing nullptr or
SkFontMgr::RefEmpty() disables font fallback rather than defaulting to system
fonts, causing any characters missing from the primary font to fail shaping.
Injecting an SkFontMgr from the application layer preserves font fallback while
avoiding hardcoded OS-specific font manager dependencies inside library modules.

## Examples

Bad:

```cpp
// Disables font fallback; characters not in the primary font fail to shape
auto shaper = SkShapers::HB::ShaperDrivenWrapper(unicode, SkFontMgr::RefEmpty());
auto iter = SkShaper::MakeFontMgrRunIterator(text, len, font, nullptr);
```

Good:

```cpp
// Accepts an injected SkFontMgr from the caller to resolve fallback fonts
void Editor::setFontMgr(sk_sp<SkFontMgr> fontMgr) {
    fFontMgr = std::move(fontMgr);
}
...
auto shaper = SkShapers::HB::ShaperDrivenWrapper(unicode, fFontMgr);
auto iter = SkShaper::MakeFontMgrRunIterator(text, len, font, fFontMgr);
```
