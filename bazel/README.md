# Skia Bazel Build System (`bazel/`)

## Overview

`bazel/` contains the Starlark rules, macros, dependency definitions, platform configurations, and project exporter tools for Skia's Bazel build system.

* **Authoritative Build Graph**: Bazel is the source of truth for Skia's modular library graph (`include/`, `src/`, `modules/`, `rust/`) and powers hermetic local and Remote Build Execution (RBE) builds via `bazelisk`.
* **Custom Starlark Rules**: `BUILD.bazel` files across Skia load custom wrappers rather than raw `@rules_cc` rules to keep compiler flags, symbol visibility, and Google-internal compatibility consistent.
* **GN Synchronization**: File additions or removals in `include/`, `src/`, and `modules/` are synced to `gn/*.gni` and `modules/*/*.gni` for the [`../gn/`](../gn/) build via `make -C bazel generate_gni`.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`common_config_settings/`](common_config_settings/) | Shared `config_setting` targets used in `select()` blocks across the repository. |
| [`device_specific_configs/`](device_specific_configs/) | Target device specifications and GPU/CPU configurations for benchmarking and testing. |
| [`exporter/`](exporter/) | Library packages that translate Bazel build graphs into GN `.gni` lists and CMake files. |
| [`exporter_tool/`](exporter_tool/) | Command-line binary (`//bazel/exporter_tool`) invoked by `make -C bazel generate_gni`. |
| [`external/`](external/) | `BUILD.bazel` definitions and configuration rules for third-party dependencies. |
| [`platform/`](platform/) | Host and target platform constraints and execution platform definitions. |
| [`rbe/`](rbe/) | Hermetic C++ toolchain definitions and RBE Docker container configurations. |
| [`user/`](user/) | Directory for developer-local `.bazelrc` overrides (`bazel/user/buildrc`). |

## Key Files and Entrypoints

The primary rules, configurations, and entrypoints in `bazel/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Core Starlark Rules** | [`skia_rules.bzl`](skia_rules.bzl), [`macros.bzl`](macros.bzl) | C++/ObjC library, binary, and test wrappers (`skia_cc_library`, `skia_cc_library_with_testutil`, `skia_objc_library`), filegroup splitters (`split_srcs_and_hdrs`), multi-select helpers (`select_multi`), and language rule re-exports. |
| **Build Flags & Presets** | [`flags.bzl`](flags.bzl), [`buildrc`](buildrc), [`devicesrc`](devicesrc) | Configurable build flags and `--config` presets (imported by the root `.bazelrc`) for toolchains, sanitizers, RBE, and target devices. |
| **Third-Party Dependencies** | [`deps.json`](deps.json), [`cipd_install.bzl`](cipd_install.bzl), [`gcs_mirror.bzl`](gcs_mirror.bzl) | Pinned git commit hashes, CIPD package definitions, and GCS mirror rules for external repositories. |
| **Project Export & Generation** | [`Makefile`](Makefile) | Convenience targets such as `make -C bazel generate_gni` to synchronize Bazel filegroups with [`../gn/`](../gn/). |

## Common Workflows

Always invoke Bazel via `bazelisk` (which reads `.bazelversion` to use the pinned Bazel release):

* **Core & GPU Modular Targets**: `bazelisk build //:core //src/gpu/ganesh:ganesh //src/gpu/graphite:graphite`
* **Full Build Smoke Test**: `bazelisk build //tools:full_build`
* **Test Runner & Viewer**: `bazelisk build //dm:dm //tools/viewer:viewer`
* **Regenerate GN File Lists**: `make -C bazel generate_gni`

