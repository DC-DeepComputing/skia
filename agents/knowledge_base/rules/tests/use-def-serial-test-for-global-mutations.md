---
name: skia/tests/use-def-serial-test-for-global-mutations
description: When writing unit tests that mutate process-wide state like locales, global caches, or environment variables, use DEF_SERIAL_TEST instead of DEF_TEST
rubric: safety
severity: should_fix
---

**Rule.** Define unit tests that mutate process-wide or global state (such as
std::setlocale, global memory handlers, shared caches, or environment variables)
using DEF_SERIAL_TEST instead of DEF_TEST.

**Why.** Skia's test runner executes DEF_TEST cases concurrently across multiple
worker threads. Mutating process-wide or global state during parallel test
execution causes data races and non-deterministic test failures in concurrently
running tests; DEF_SERIAL_TEST marks tests as kCPUSerial, ensuring they run in
isolation before parallel worker execution begins.

## Examples

Bad:

```cpp
DEF_TEST(JsonParse_CustomLocale, reporter) {
    ScopedLocale commaLocale("de_DE.UTF-8");
    // Parses numbers using process-wide locale setting
    test_parse_numbers(reporter);
}
```

Good:

```cpp
DEF_SERIAL_TEST(JsonParse_CustomLocale, reporter) {
    ScopedLocale commaLocale("de_DE.UTF-8");
    // Runs serially before parallel CPU tests to avoid races
    test_parse_numbers(reporter);
}
```
