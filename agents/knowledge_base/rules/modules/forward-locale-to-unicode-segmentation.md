---
name: skia/modules/forward-locale-to-unicode-segmentation
description: When implementing or wrapping Unicode text segmentation and boundary helpers, pass caller-specified locale arguments through to underlying SkUnicode break methods rather than ignoring them
rubric: testing
severity: should_fix
---

**Rule.** When defining or wrapping Unicode text segmentation and boundary
helpers (such as word, line, or grapheme break iterators via SkUnicode), forward
any caller-specified locale parameter to the underlying SkUnicode segmentation
methods (such as getWords or makeBreakIterator) rather than omitting it or
passing nullptr.

**Why.** Unicode text segmentation algorithms (such as word and line breaking in
ICU) depend on locale-specific dictionary analysis for languages such as Thai,
Japanese, and Chinese. Dropping or ignoring a caller-supplied locale parameter
causes segmentation to fall back to the root or system locale, producing
incorrect boundary positions for localized text.

## Examples

Bad:

```cpp
std::vector<bool> GetUtf8WordBoundaries(const char* begin, size_t byteCount, const char* locale) {
    auto unicode = get_unicode();
    if (!unicode) {
        return {};
    }
    std::vector<SkUnicode::Position> positions;
    // Bad: Ignores the caller-specified locale parameter
    if (!unicode->getWords(begin, byteCount, &positions) || byteCount == 0) {
        return {};
    }
    ...
}
```

Good:

```cpp
std::vector<bool> GetUtf8WordBoundaries(const char* begin, size_t byteCount, const char* locale) {
    auto unicode = get_unicode();
    if (!unicode) {
        return {};
    }
    std::vector<SkUnicode::Position> positions;
    // Good: Forwards the caller-specified locale to SkUnicode segmentation
    if (!unicode->getWords(begin, byteCount, locale, &positions) || byteCount == 0) {
        return {};
    }
    ...
}
```
