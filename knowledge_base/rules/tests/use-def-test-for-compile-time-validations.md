---
name: skia/tests/use-def-test-for-compile-time-validations
description: When writing compile-time validations or type checks in unit tests, register them using DEF_TEST instead of uncalled static functions marked [[maybe_unused]]
rubric: readability
severity: should_fix
---

**Rule.** When authoring compile-time checks, template concept verifications, or
type validity tests in Skia unit test files, register them as test cases using
DEF_TEST. Do not define uncalled standalone static functions marked with
[[maybe_unused]] to verify that types or operations compile cleanly.

**Why.** Standalone uncalled functions marked [[maybe_unused]] are not executed
by Skia's test runner (dm), do not report test results, and risk being deleted
as dead code during future refactorings. Registering them via DEF_TEST
integrates them into the test harness, signals their intent as active regression
tests, and guarantees they run in CI.

## Examples

Bad:

```cpp
// Uncalled static function marked [[maybe_unused]] to silence compiler warnings
[[maybe_unused]] static void testRequiredPropertiesCanBeAKey() {
    std::map<SkImage::RequiredProperties, int> test;
    SkImage::RequiredProperties rp;
    test.emplace(rp, 7);
}
```

Good:

```cpp
// Registered with Skia's dm test harness via DEF_TEST
DEF_TEST(SkImageRequiredPropertiesCanBeMapKey, r) {
    std::map<SkImage::RequiredProperties, int> test;
    SkImage::RequiredProperties rp;
    test.emplace(rp, 7);
}
```
