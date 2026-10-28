---
name: skia/build-gn/suffix-test-targets-with-tests
description: When defining GN build targets containing test utilities, harnesses, or test cases, suffix target names with _tests to distinguish them from production libraries
rubric: readability
severity: should_fix
---

**Rule.** When defining GN build targets (such as `skia_source_set`,
`source_set`, or `optional`) that contain test utilities, test harnesses, or
test cases, suffix the target name with `_tests` (or name module-local targets
`tests`) rather than using a bare component name. Do not use bare names for
test-only targets, and mark test helper targets with `testonly = true`.

**Why.** Bare target names imply that the target compiles production
functionality or a core library component. Suffixing test targets and test
utility sets with `_tests` makes their test-only role immediately explicit in
build files and dependency lists, preventing accidental production dependencies.

## Examples

Bad:

```cpp
optional("fontscanner") {
  enabled = skia_use_freetype || skia_use_fontations
  sources = [
    "tests/FontScanner.cpp",
    "tests/FontScanner.h",
  ]
}
```

Good:

```cpp
skia_source_set("fontscanner_tests") {
  testonly = true
  deps = [ ":skia" ]
  sources = [
    "tests/FontScanner.cpp",
    "tests/FontScanner.h",
  ]
}
```
