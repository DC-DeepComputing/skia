---
name: skia/modules/instantiate-supported-backends-explicitly-in-tests
description: When testing features requiring specific backend implementations, explicitly instantiate each supported backend in separate test cases rather than using compound preprocessor guards
rubric: testing
severity: should_fix
---

**Rule.** When testing features supported only by specific backends (such as
Unicode or text shaping engines), do not guard a single generic test with
compound preprocessor checks (`#if defined(IMPL_A) || defined(IMPL_B)`).
Instead, explicitly instantiate and exercise each supported backend
implementation in dedicated, separately guarded test cases (e.g., using
backend-specific test definitions or factory helpers).

**Why.** Multiple backend implementations (such as ICU, ICU4X, and libgrapheme)
can be enabled concurrently in the same build configuration. Guarding a single
test with compound `||` checks fails to verify both backends, and if default or
ambient factories select an unsupported or different backend, the test will
either fail spuriously or leave supported backends completely unexercised.

## Examples

Bad:

```cpp
// Bad: Compound preprocessor guard fails to exercise both backends and may run against an unsupported default backend.
#if defined(SK_UNICODE_ICU_IMPLEMENTATION) || defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_EmojiFontResolution, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    // Relies on ambient default SkUnicode, which may be libgrapheme or only one of ICU/ICU4X
    ...
}
#endif
```

Good:

```cpp
// Good: Define dedicated test cases that explicitly instantiate each supported backend implementation.
static void SkParagraph_EmojiFontResolution(sk_sp<SkUnicode> unicode, skiatest::Reporter* reporter) {
    ...
}

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU_EmojiFontResolution, reporter) {
    SkParagraph_EmojiFontResolution(SkUnicodes::ICU::Make(), reporter);
}
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU4X_EmojiFontResolution, reporter) {
    SkParagraph_EmojiFontResolution(SkUnicodes::ICU4X::Make(), reporter);
}
#endif
```
