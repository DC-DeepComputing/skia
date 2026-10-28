---
name: skia/build-gn/mark-test-targets-testonly
description: When defining test-only targets or test helper source sets in GN or Bazel, mark them with testonly = true to prevent production dependencies
rubric: design
severity: should_fix
---

**Rule.** Mark all test-only build targets (such as unit test suites, test
fixtures, fakes, or test helper source sets) with `testonly = true` in GN or
`testonly = True` in Bazel. Do not define test support targets without this
declaration, even if they are only consumed internally within test files.

**Why.** Setting `testonly` enforces build visibility and dependency layering at
the build system level, ensuring that production libraries and binaries cannot
inadvertently depend on test-only code, test mocks, or test frameworks.

## Examples

Bad:

```cpp
skia_source_set("fontscanner_tests") {
  deps = [ ":skia" ]
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
