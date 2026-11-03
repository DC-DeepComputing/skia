# Skia Unit & GPU Tests (`tests/`)

## Overview

`tests/` contains Skia's C++ unit tests, CPU algorithm checks, and GPU backend behavioral assertions for both Ganesh and Graphite.

* **Programmatic Assertions**: Unlike visual Golden Master tests in [`../gm/`](../gm/), tests in `tests/` use programmatic checks (`REPORTER_ASSERT`, `ERRORF`) to verify API contracts, edge cases, data structures, resource caching, shader compilation, and GPU context lifecycles.
* **CPU & GPU Execution Modes**: Supports parallel CPU unit tests (`DEF_TEST`), main-thread serial CPU tests (`DEF_SERIAL_TEST`), and backend-looping GPU tests (`DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS`, `DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS`).
* **Android CTS Enforcement**: GPU test macros take a `CtsEnforcement` policy specifying which Android API levels enforce the test in SkQP (`CtsEnforcement::kNever`, `CtsEnforcement::kApiLevel_*`, `CtsEnforcement::kNextRelease`).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`ganesh/`](ganesh/) | Shared Ganesh GPU test utilities (`GaneshTestUtils.h`; Ganesh unit tests live in the top-level `tests/` directory). |
| [`graphite/`](graphite/) | Graphite GPU unit tests, including [`graphite/precompile/`](graphite/precompile/) and [`graphite/sparse_strips/`](graphite/sparse_strips/). |
| [`sksl/`](sksl/) | Golden `skslc`-compiled shader outputs (SPIR-V, Metal, WGSL, GLSL, SkRP) generated from `resources/sksl/`. |

## Key Classes and Entrypoints

The primary test harness headers and utilities in `tests/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Test Registration & Reporter** | [`Test.h`](Test.h), [`Test.cpp`](Test.cpp), [`TestType.h`](TestType.h) | Defines `skiatest::Test`, `skiatest::Reporter`, `skiatest::ReporterContext`, assertion macros (`REPORTER_ASSERT`, `ERRORF`, `INFOF`), and registration macros (`DEF_TEST`, `DEF_CONDITIONAL_TEST`, `DEF_SERIAL_TEST`, `DEF_GANESH_TEST*`, `DEF_GRAPHITE_TEST*`). |
| **Android CTS Enforcement** | [`CtsEnforcement.h`](CtsEnforcement.h), [`CtsEnforcement.cpp`](CtsEnforcement.cpp) | `CtsEnforcement` policy class controlling whether and at what Android API level a GPU test runs in SkQP. |
| **Test Harness & Pixel Helpers** | [`TestHarness.h`](TestHarness.h), [`ComparePixels.h`](ComparePixels.h), [`GaneshTestUtils.h`](GaneshTestUtils.h), [`RecordTestUtils.h`](RecordTestUtils.h) | Identifies the active test runner (`CurrentTestHarness`), compares pixel buffers with tolerance, and provides shared Ganesh and `SkRecord` inspection helpers. |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../dm/`](../dm/) | Primary test runner that executes all registered `skiatest::Test` suites. |
| [`../gm/`](../gm/) | Golden Master rendering tests for visual pixel comparison in Skia Gold. |
| [`../tools/`](../tools/) | GPU test context factories (`gpu/`, `ganesh/`, `graphite/`), test fonts (`fonts/`), and resource loaders. |

