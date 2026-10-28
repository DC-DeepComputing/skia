---
name: skia/modules/use-test-fonts-in-skparagraph-tests
description: In SkParagraph unit tests, define test cases using UNIX_ONLY_TEST rather than DEF_TEST, and use test-only fonts and structural assertions
rubric: correctness
severity: blocking
---

**Rule.** In SkParagraph unit tests, define test cases using UNIX_ONLY_TEST
rather than DEF_TEST to restrict execution to Unix environments with
deterministic font support. Additionally, instantiate ResourceFontCollection
with testOnly=true (or configure a deterministic test font manager) to prevent
fallback to system fonts, and verify structural properties or non-emptiness
rather than asserting exact platform-dependent line counts or pixel coordinates.

**Why.** Text shaping, font fallback, and glyph metrics vary across operating
systems and native-font bot configurations (such as Ubuntu-NativeFonts). Using
DEF_TEST executes tests on Windows and macOS where font availability and shaping
results are not guaranteed, while omitting testOnly=true or asserting exact line
counts causes non-deterministic failures when system fonts resolve.

## Examples

Bad:

```cpp
// Bad: Uses DEF_TEST instead of UNIX_ONLY_TEST, risking Windows/macOS font failures
DEF_TEST(SkParagraph_EndWithLineSeparator, reporter) {
    sk_sp<FontCollection> fontCollection = sk_make_sp<FontCollection>();
    // ...
}

// Bad: Omits testOnly=true and asserts exact line count vulnerable to native font metrics
UNIX_ONLY_TEST(SkParagraph_SoftHyphenAtLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    paragraph->layout(60);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
}
```

Good:

```cpp
// Good: Uses UNIX_ONLY_TEST to restrict execution to Unix environments
UNIX_ONLY_TEST(SkParagraph_EndWithLineSeparator, reporter) {
    sk_sp<FontCollection> fontCollection = sk_make_sp<FontCollection>();
    // ...
}

// Good: Passes testOnly=true and asserts structural property (non-emptiness)
UNIX_ONLY_TEST(SkParagraph_SoftHyphenAtLineBreak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(/*testOnly=*/true);
    paragraph->layout(80);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->lines().empty());
}
```
