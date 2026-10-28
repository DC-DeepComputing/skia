---
name: skia/build-gn/match-preprocessor-defines-to-source-guards
description: When guarding code or tests behind preprocessor macros, ensure the identifier is explicitly defined in build configurations and matches source guards exactly
rubric: testing
severity: should_fix
---

**Rule.** When introducing, renaming, or guarding feature code, test fixtures,
or test cases behind preprocessor flags, verify that the macro is explicitly
defined in the relevant build configuration target (`defines` or
`public_defines` in `BUILD.gn`, `BUILD.bazel`, or `*.gni`) and that its
identifier matches source `#if defined(...)` or `#ifdef` guards exactly. Do not
leave macro guards pointing to undefined identifiers or commented-out source
`#define` directives, and ensure flag updates are applied across all build
manifests and corresponding source files simultaneously.

**Why.** Preprocessor guards checking undefined or mismatched macro names
silently evaluate to false at compile time without producing compiler errors. A
missing definition or naming mismatch between build manifests and source code
leaves the guarded implementation or test case permanently inactive and
unexercised during builds and CI runs.

## Examples

Bad:

```cpp
// 1. Build define mismatch:
// BUILD.gn defines one name:
optional("png_encode") {
  public_defines = [ "SK_PNG_ENCODER_USE_CONVERT_PIXELS" ]
}
// But C++ source checks a different name:
#ifdef SK_CODEC_ENCODES_PNG_WITH_CONVERT_PIXELS
    // Never compiled due to mismatched macro names.
#endif

// 2. Undefined or commented-out test guard:
//#define SKUNICODE_TEST_ICU
#ifdef SKUNICODE_TEST_ICU
class SkUnicode_test : public SkUnicode { ... };  // Silently skipped in CI
#endif
```

Good:

```cpp
// 1. Build define matches source guard:
optional("png_encode") {
  public_defines = [ "SK_CODEC_ENCODES_PNG_WITH_CONVERT_PIXELS" ]
}
#ifdef SK_CODEC_ENCODES_PNG_WITH_CONVERT_PIXELS
    // Compiles and runs as expected.
#endif

// 2. Explicitly configured test target define:
// In BUILD.gn:
// if (paragraph_tests_unicode) { defines = [ "SKUNICODE_TEST_AVAILABLE" ] }
#ifdef SKUNICODE_TEST_AVAILABLE
class SkUnicode_test : public SkUnicode { ... };  // Enabled in build config
#endif
```
