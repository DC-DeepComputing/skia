---
name: skia/sksl/add-sksl-tests-for-new-stage-intrinsics
description: When modifying SkSL parsing, stage intrinsics, or AST ranges, add test shaders, update build files, and re-generate golden outputs via skia_compile_sksl_tests=true
rubric: correctness
severity: should_fix
---

**Rule.** When adding or modifying SkSL parsing, compiler error handling,
language features, or stage intrinsics (such as enabling texture operations in
fragment or compute shaders), add standalone test .sksl shaders under
resources/sksl/ (such as resources/sksl/errors/ or resources/sksl/intrinsics/)
and register them in both resources/sksl/BUILD.bazel and gn/sksl_tests.gni.
Additionally, when modifying the parser, compiler diagnostics, or AST node
source ranges (via helpers like rangeFrom()), re-generate and commit all updated
compiler golden outputs under tests/sksl/ by building with
skia_compile_sksl_tests=true in GN args. Verify that test shaders compile
cleanly or produce expected error underline spans across all active backends.

**Why.** SkSL language features, parser error reporting, and backend code
generators rely on committed golden outputs in tests/sksl/ to verify code
generation and detect regressions in diagnostic underline spans. Registering
test shaders across both Bazel and GN ensures automated tests run consistently,
while re-generating golden files keeps test expectations in sync with compiler
AST source range adjustments.

## Examples

Bad:

```cpp
// 1. Enabling intrinsics without test shaders or build registration:
// src/sksl/sksl_gpu.sksl
$pure half4 textureRead(sampler2D s, uint2 pos);

// 2. Modifying AST source ranges without re-generating golden test expectations:
// tests/sksl/errors/InterfaceBlockStorageModifiers.glsl (outdated underline span)
error: 1: modifier 'out' is not permitted on an interface block field
uniform foo { out int x; };
              ^^^^^^^^^^
1 error
```

Good:

```cpp
// Register new tests in resources/sksl/BUILD.bazel and gn/sksl_tests.gni:
skia_filegroup(name = "sksl_error_tests", srcs = ["errors/NewError.sksl"])
sksl_error_tests = ["errors/NewError.sksl"]

// Re-generate tests/sksl/ golden outputs using skia_compile_sksl_tests=true:
// tests/sksl/errors/InterfaceBlockStorageModifiers.glsl
error: 1: modifier 'out' is not permitted on an interface block field
uniform foo { out int x; };
              ^^^^^^^^^
1 error
```
