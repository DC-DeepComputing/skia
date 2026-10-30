# Skia Unit & GPU Tests (`tests/`)

## Overview & Scope
`tests/` contains Skia's C++ unit tests, CPU algorithm checks, and GPU backend behavior assertions (for both Ganesh and Graphite). Unlike Golden Master (`gm/`) rendering tests—which compare visual pixel output in Skia Gold—tests in `tests/` use programmatic assertions (`REPORTER_ASSERT`, `ERRORF`) to verify API contracts, edge cases, data structures, resource caching, shader compilation, and GPU context lifecycles.

## Key Concepts & Entrypoints

### Test Definition Macros (`tests/Test.h`)
All tests include [`tests/Test.h`](Test.h) and register themselves into `skiatest::TestRegistry` using macro entrypoints:

* **CPU Unit Tests**:
  * `DEF_TEST(Name, reporter)`: Standard CPU unit test executed in parallel by the test runner.
  * `DEF_CONDITIONAL_TEST(Name, reporter, condition)`: CPU test that only runs when `condition` evaluates to true.
  * `DEF_SERIAL_TEST(Name, reporter)`: CPU test forced to run serially on the main thread (use when modifying global state or testing thread-unsafe singletons).
* **Ganesh GPU Tests**:
  * `DEF_GANESH_TEST(Name, reporter, options, ctsEnforcement)`: Standalone Ganesh test that manages its own `GrDirectContext` or takes `GrContextOptions`.
  * `DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Name, reporter, context_info, ctsEnforcement)`: Runs the test across all compiled-in, real rendering Ganesh backends (`GL`, `Vulkan`, `Metal`, `D3D`, `Dawn`).
  * `DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Name, reporter, context_info, ctsEnforcement)`: Runs across all Ganesh contexts, including mock/non-rendering contexts.
  * Backend-specific variants: `DEF_GANESH_TEST_FOR_GL_CONTEXT`, `DEF_GANESH_TEST_FOR_VULKAN_CONTEXT`, `DEF_GANESH_TEST_FOR_METAL_CONTEXT`, `DEF_GANESH_TEST_FOR_MOCK_CONTEXT`.
* **Graphite GPU Tests**:
  * `DEF_GRAPHITE_TEST(Name, reporter, ctsEnforcement)`: Standalone Graphite test without an automatic context loop.
  * `DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Name, reporter, graphite_context, ctsEnforcement)`: Runs across all compiled-in Graphite rendering backends.
  * `DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(Name, reporter, graphite_context, ctsEnforcement)`: Runs across all Graphite contexts.
  * Full-control & backend-specific variants: `DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS` (exposes `skiatest::graphite::GraphiteTestContext*` and `TestOptions`), `DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT`, `DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT`, `DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT`.

> **Note on `CtsEnforcement`**: GPU test macros require a [`CtsEnforcement`](CtsEnforcement.h) parameter (`CtsEnforcement::kNever`, `CtsEnforcement::kApiLevel_T`, `CtsEnforcement::kNextRelease`, etc.) to specify whether and when the test is enforced in Android's Compatibility Test Suite (SkQP). New internal or non-Android tests typically pass `CtsEnforcement::kNever`.

### Assertions & `skiatest::Reporter`
Tests receive a `skiatest::Reporter*` used to record failures and diagnostic context:
* `REPORTER_ASSERT(reporter, cond, ...)`: Asserts that `cond` is true; on failure, logs the file/line, condition string, and optional `SkStringPrintf`-style formatted message without aborting the function automatically.
* `ERRORF(reporter, ...)`: Unconditionally reports a formatted test failure.
* `INFOF(reporter, ...)`: Prints formatted output only when verbose mode (`-V` / `--veryVerbose`) is enabled.
* `skiatest::ReporterContext ctx(reporter, "subtest_name")`: RAII helper that pushes a context string onto the reporter stack so any failure inside the scope includes `"subtest_name"`.

## Directory Structure
* `tests/*.cpp`: Core, codec, pathops, PDF, SkSL, and legacy Ganesh unit tests.
* [`tests/graphite/`](graphite/): Graphite GPU tests, including [`precompile/`](graphite/precompile/) and [`sparse_strips/`](graphite/sparse_strips/) subdirectories.
* [`tests/ganesh/`](ganesh/): Shared Ganesh test utilities (`GaneshTestUtils.h/.cpp`).
* [`tests/sksl/`](sksl/): Golden `skslc`-compiled shader outputs (SPIR-V, Metal, WGSL, GLSL, SkRP) generated from `/resources/sksl/` (see [`tests/sksl/README.txt`](sksl/README.txt)).

## Adding & Running Tests

### 1. Registering a New Test File
When adding a new test file, register it in **both** build systems:
1. **GN (`gn/tests.gni`)**: Add the file path to the appropriate list in [`gn/tests.gni`](../gn/tests.gni) (e.g., `tests_sources`, `pathops_tests_sources`, `ganesh_tests_sources`, `graphite_tests_sources`, or backend-specific lists like `graphite_dawn_tests_sources`).
2. **Bazel (`tests/BUILD.bazel`)**: Add the file to the corresponding `filegroup` in [`tests/BUILD.bazel`](BUILD.bazel) (`tests_sources`, `ganesh_tests_sources`, `graphite_tests_sources`, etc.).

### 2. Running Tests
* **Via `dm` (GN/Ninja or Bazel)**:
  ```sh
  # Run a specific unit test by name (--match / -m)
  out/Debug/dm --src tests --match ^MyTestName$

  # Run CPU-only unit tests quickly (skipping GPU test contexts)
  out/Debug/dm --src tests --nogpu -m MyTestName
  ```
* **Via Bazelisk**:
  ```sh
  bazelisk run //dm -- --src tests --match ^MyTestName$
  ```
