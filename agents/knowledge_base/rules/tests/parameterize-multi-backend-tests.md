---
name: skia/tests/parameterize-multi-backend-tests
description: When testing interfaces or modules with multiple backends, parameterize tests across every enabled backend rather than testing only the first available fallback
rubric: testing
severity: should_fix
---

**Rule.** When testing interfaces or modules that support multiple backend
implementations (such as SkUnicode's ICU, ICU4X, and libgrapheme backends, or
GPU rendering contexts), parameterize unit tests across every enabled backend
rather than selecting only the first available backend via fallback logic. Use
backend-parameterized test macros (such as DEF_TEST_UNICODES or
DEF_GANESH_TEST_FOR_ALL_CONTEXTS), iterate over enabled backend factories, or
extract shared test logic into a helper function and declare separate test cases
(DEF_TEST / UNIX_ONLY_TEST) for each enabled backend.

**Why.** Using an #if or cascading fallback helper to obtain a single test
instance only runs tests on the first enabled backend, leaving alternative
implementations completely untested in builds where multiple backends are
compiled. Parameterizing tests to instantiate and run against each enabled
backend ensures comprehensive test coverage and prevents backend-specific
regressions.

## Examples

Bad:

```cpp
// Bad: Returns only the first available backend via cascading fallback.
std::unique_ptr<SkUnicode> get_icu_based_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    return SkUnicode::MakeIcuBasedUnicode();
#elif defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    return SkUnicode::MakeIcu4xBasedUnicode();
#endif
    return nullptr;
}

UNIX_ONLY_TEST(SkParagraph_EmojiRuns, reporter) {
    auto icu = get_icu_based_unicode();
    REPORTER_ASSERT(reporter, icu);
    ...
}
```

Good:

```cpp
// Good: Extract test logic into a helper and define separate test wrappers for each enabled backend.
static void SkParagraph_EmojiRuns(SkUnicode* icu, skiatest::Reporter* reporter) {
    ...
}

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_EmojiRuns_ICU, reporter) {
    auto icu = SkUnicode::MakeIcuBasedUnicode();
    REPORTER_ASSERT(reporter, icu);
    SkParagraph_EmojiRuns(icu.get(), reporter);
}
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_EmojiRuns_ICU4X, reporter) {
    auto icu4x = SkUnicode::MakeIcu4xBasedUnicode();
    REPORTER_ASSERT(reporter, icu4x);
    SkParagraph_EmojiRuns(icu4x.get(), reporter);
}
#endif
```
