# Skia Bazel Build System (`bazel/`)

## Overview & Scope
`bazel/` contains the Starlark rules, macros, dependency definitions, platform configurations, and project exporter tools for Skia's **Bazel** build system. Bazel is the authoritative source of truth for Skia's modular library graph (`include/`, `src/`, `modules/`, `rust/`) and powers hermetic local and Remote Build Execution (RBE) builds via `bazelisk`.

## Key Concepts & Entrypoints

### Starlark Rules & Macros
`BUILD.bazel` files across Skia load custom wrappers rather than raw `@rules_cc` rules so compiler flags, symbol visibility, and Google-internal (G3) compatibility stay consistent:
* [`bazel/skia_rules.bzl`](skia_rules.bzl): Core Starlark rules and helpers requiring zero third-party workspace dependencies:
  * `skia_cc_library`: Wraps `cc_library` with `DEFAULT_COPTS` and `local_defines = ["SKIA_IMPLEMENTATION=1"]`.
  * `skia_cc_binary` / `skia_cc_test`: Wraps `cc_binary` / `cc_test` with Skia's default compiler and linker options.
  * `skia_objc_library`: Wraps `objc_library` with platform-aware `ios_frameworks` / `mac_frameworks` and `non_arc_srcs` support.
  * `skia_cc_library_with_testutil` / `skia_objc_library_with_testutil`: Generates both a production target (`<name>`) and a test-only target (`<name>_TEST_UTIL`) compiled with `-DGPU_TEST_UTILS` and rewritten `_TEST_UTIL` dependencies.
  * `split_srcs_and_hdrs(name, files)`: Splits a mixed list of `.cpp`/`.mm`/`.h` files into `<name>_srcs` and `<name>_hdrs` `skia_filegroup`s (used heavily for GNI export).
  * `select_multi(values_map)`: Unrolls a dictionary into multiple independent `select()` statements so additive features (e.g., multiple codecs) can be enabled simultaneously.
  * `supports_platforms(*platforms)`: Convenience helper for `target_compatible_with`.
* [`bazel/macros.bzl`](macros.bzl): Re-exports `skia_rules.bzl` macros alongside Go, Python, Gazelle, and WebAssembly rules (`go_binary`, `py_binary`, `wasm_cc_binary`, `bool_flag`, `string_flag_with_values`).
* [`bazel/flags.bzl`](flags.bzl) & [`bazel/common_config_settings/`](common_config_settings/): Defines configurable build flags and `config_setting` targets used in `select()` blocks.
* [`bazel/buildrc`](buildrc) & [`bazel/devicesrc`](devicesrc): Standard `--config` presets (imported by the root `.bazelrc`) for toolchains, sanitizers, RBE, and target devices.

### Exporting Bazel Targets to GN (`bazel/exporter_tool/`)
Because `BUILD.bazel` files in `include/`, `src/`, and `modules/` are the source of truth for file lists, any file additions or removals in those directories must be synced to `gn/*.gni` and `modules/*/*.gni` for the [GN build](../gn/README.md) by running `make -C bazel generate_gni` (which builds and invokes `//bazel/exporter_tool` and formats the output with `gn format`).

## Subdirectory Documentation
Refer to the existing child READMEs for detailed workflows:
* [`bazel/exporter_tool/README.md`](exporter_tool/README.md) & [`bazel/exporter/build_proto/README.md`](exporter/build_proto/README.md): How Bazel targets map to `gn/*.gni` file lists and `CMakeLists.txt`.
* [`bazel/external/README.md`](external/README.md): How third-party dependencies in `bazel/external/` and [`bazel/deps.json`](deps.json) are configured and why `includes` (`-isystem`) is used instead of `strip_include_prefix`.
* [`bazel/rbe/README.md`](rbe/README.md): Hermetic toolchain setup, RBE Docker container management, and SLSA reproducibility.
* [`bazel/user/README.md`](user/README.md): How to define personal `.bazelrc` overrides in `bazel/user/buildrc`.

## Common Bazelisk Targets
Always invoke Bazel via `bazelisk` (which reads `.bazelversion` to use the pinned Bazel release):
* `//:core`, `//src/gpu/ganesh:ganesh`, `//src/gpu/graphite:graphite`: Core and GPU modular library targets.
* `//tools:full_build`: Smoke-test target compiling all public/internal headers and modular targets.
* `//dm:dm`, `//tools/viewer:viewer`: Main test runner and interactive viewer binaries.
