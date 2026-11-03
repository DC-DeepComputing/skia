# Skia GN Build Configuration (`gn/`)

## Overview

`gn/` contains the build configuration, toolchain definitions, global build arguments, and `.gni` source-file lists for Skia's GN (Generate Ninja) build system.

* **Global Configuration & Toolchains**: Detects host/target platforms, configures compilers and linkers, and declares `skia_*` build arguments and target wrapper templates.
* **Generated vs. Manual `.gni` Source Lists**: Core library and SkSL `.gni` files carry `# DO NOT EDIT: This is a generated file.` and are exported from [`../bazel/`](../bazel/) via `make -C bazel generate_gni`, whereas test, GM, benchmark, and fuzz `.gni` lists are maintained directly.
* **Downstream Project Generators**: Provides Python scripts for translating GN build descriptions into Android Soong and CMake build files and running build-time SkSL compilation actions.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`skia/`](skia/) | Low-level compiler and linker `config()` blocks (`default`, `optimize`, `warnings`, `debug_symbols`). |
| [`toolchain/`](toolchain/) | Toolchain definitions (`gcc_like`, `msvc`, `wasm`) and compile/link rules. |

## Key Files and Entrypoints

The primary configuration files, source lists, and helper scripts in `gn/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Core GN Configuration** | [`BUILDCONFIG.gn`](BUILDCONFIG.gn), [`skia.gni`](skia.gni) | Global GN entrypoint (`BUILDCONFIG.gn`) that detects OS/CPU and selects toolchains, and `skia.gni` which declares `skia_*` arguments and target templates (`skia_executable`, `skia_source_set`, `skia_component`). |
| **Auto-Generated `.gni` Lists** | [`core.gni`](core.gni), [`gpu.gni`](gpu.gni), [`graphite.gni`](graphite.gni), [`effects.gni`](effects.gni), [`codec.gni`](codec.gni), [`ports.gni`](ports.gni), [`sksl.gni`](sksl.gni), [`utils.gni`](utils.gni) | Generated from `BUILD.bazel` files via `make -C bazel generate_gni` (alongside [`effects_imagefilters.gni`](effects_imagefilters.gni), [`pathops.gni`](pathops.gni), [`pdf.gni`](pdf.gni), [`rust.gni`](rust.gni), [`sksl_tests.gni`](sksl_tests.gni), [`svg.gni`](svg.gni), [`xml.gni`](xml.gni), and [`xps.gni`](xps.gni)). Do not hand-edit. |
| **Manually Maintained `.gni` Lists** | [`tests.gni`](tests.gni), [`gm.gni`](gm.gni), [`bench.gni`](bench.gni), [`fuzz.gni`](fuzz.gni), [`opts.gni`](opts.gni) | Source lists for unit tests, GMs (`make_gm_gni.py`), benchmarks, fuzzers, and architecture-specific SIMD targets edited directly alongside `BUILD.bazel`. |
| **Build & Export Scripts** | [`gn_to_bp.py`](gn_to_bp.py), [`gn_to_cmake.py`](gn_to_cmake.py), [`compile_sksl_tests.py`](compile_sksl_tests.py), [`minify_sksl.py`](minify_sksl.py) | Translates GN build graphs into Android `Android.bp` and `CMakeLists.txt`, and runs `skslc` / `sksl-minify` build actions. |

