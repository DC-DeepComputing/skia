---
name: skia/modules/allow-deferred-builder-dependencies
description: When implementing or consuming builder dependencies, allow nullptr at construction for deferred injection and handle optional dependencies gracefully without asserting
rubric: correctness
severity: should_fix
---

**Rule.** When implementing builder factories that support deferred dependency
injection via setters (such as ParagraphBuilder::make with SetUnicode), allow
nullptr dependencies at construction and validate required dependencies at
Build() time. When consuming optional builder dependencies (such as SkFontMgr or
ResourceProvider), do not assert non-nullness with SkASSERT or CHECK; guard
operations with null checks, provide empty fallbacks, or log an error if an
optional dependency is missing.

**Why.** Embedders often instantiate builders before providers are fully
initialized, and may omit optional dependencies entirely when features like text
or external assets are unused or handled externally. Rejecting nullptr in
factories breaks deferred configuration workflows, while asserting non-nullness
for omitted optional dependencies causes debug builds to abort and production
builds to crash with null pointer dereferences.

## Examples

Bad:

```cpp
// Bad: Rejecting nullptr in builder factory when deferred injection is supported
std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style,
        sk_sp<FontCollection> fonts,
        sk_sp<SkUnicode> unicode) {
    if (!unicode) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fonts), std::move(unicode));
}

// Bad: Asserting non-null for optional builder dependencies during execution
if (!finfo->fTypeface) {
    SkASSERT(fFontMgr);
    finfo->fTypeface = fFontMgr->makeFromData(
            fResourceProvider->loadFont(name.c_str(), finfo->fPath.c_str()));
}
```

Good:

```cpp
// Good: Allow nullptr at construction; validate required dependencies at Build()
std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style,
        sk_sp<FontCollection> fonts,
        sk_sp<SkUnicode> unicode) {
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fonts), std::move(unicode));
}
std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    SkASSERT_RELEASE(fUnicode);
    // Proceed with construction...
}

// Good: Null-check optional dependencies and handle omission gracefully with error logs
if (!finfo->fTypeface && fFontMgr) {
    finfo->fTypeface = fFontMgr->makeFromData(
            fResourceProvider->loadFont(name.c_str(), finfo->fPath.c_str()));
}
if (!finfo->fTypeface && !fFontMgr) {
    this->log(Logger::Level::kError, nullptr,
              "Could not load typeface for %s|%s because no SkFontMgr provided.",
              finfo->fFamily.c_str(), finfo->fStyle.c_str());
}
```
