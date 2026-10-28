---
name: skia/build-gn/compile-optional-tests-via-sources-for-tests
description: When configuring test targets in GN, isolate backend tests under sources_for_tests and avoid gating generic or module test suites on concrete backend flags
rubric: design
severity: blocking
---

**Rule.** Do not add test files depending on optional codecs or backends
unconditionally to `tests_sources` in `gn/tests.gni`, nor bundle backend tests
into unrelated targets with `#ifdef`s; attach them to each backend's
`sources_for_tests` under `optional(...)` in `BUILD.gn`. Furthermore, do not
gate generic test helpers or abstracted module test suites (e.g.,
`modules/skshaper`) on concrete backend implementation flags (such as
`skia_use_harfbuzz`). Keep generic test helpers as unconditional
`skia_source_set(..., testonly = true)` targets without backend dependencies,
and control module test targets strictly via the module's own test flag.

**Why.** Attaching isolated backend-specific tests to each optional target's
`sources_for_tests` ensures they compile and run only when their required
third-party dependencies are enabled. Gating abstracted module test suites or
generic helpers on concrete backend implementation flags leaks backend details
into higher layers, couples test targets to specific implementations, and
defeats the purpose of abstracting subsystems behind generic interfaces.

## Examples

Bad:

```cpp
# Gating abstracted module test target on concrete backend flags
if (skia_enable_skshaper_tests && skia_use_harfbuzz &&
    skia_enable_skunicode) {
  testonly = true
  sources = skia_shaper_tests
  deps = [
    ":skshaper",
    "../..:skia",
    "../..:test",
  ]
}

# Wrapping generic test helper in optional() with backend flags
optional("fontscanner") {
  enabled = skia_use_freetype || skia_use_fontations
  sources_for_tests = [ "tests/FontScanner_FreeTypeTest.cpp" ]
}
```

Good:

```cpp
# Gate module test targets strictly on the module's own test flag
if (skia_enable_skshaper_tests) {
  testonly = true
  sources = skia_shaper_tests
  deps = [
    ":skshaper",
    "../..:skia",
    "../..:test",
  ]
}

# Keep generic helpers unconditional; attach backend tests to sources_for_tests
skia_source_set("fontscanner_tests") {
  testonly = true
  sources = [ "tests/FontScanner.cpp" ]
}

optional("typeface_freetype") {
  sources_for_tests = [ "tests/FontScanner_FreeTypeTest.cpp" ]
}
```
