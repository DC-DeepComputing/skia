# Skia GN Build Configuration (`gn/`)

## Overview & Scope
`gn/` contains the build configuration, toolchain definitions, global build arguments, and `.gni` source-file lists for Skia's **GN (Generate Ninja)** build system, working in tandem with the root [`BUILD.gn`](../BUILD.gn). While Skia is migrating its authoritative build definition to [Bazel](../bazel/README.md), GN remains heavily used for local C++ development, `dm`/`viewer`/`nanobench` builds, and downstream integrations (such as Chromium and Android).

## Key Concepts & Entrypoints

### Core Configuration Files
* [`gn/BUILDCONFIG.gn`](BUILDCONFIG.gn): The global GN entrypoint (referenced by `.gn`). Detects host/target OS and CPU (`is_linux`, `is_mac`, `is_win`, `is_android`, `is_ios`, `is_wasm`), configures compilers (`cc`, `cxx`, `is_clang`, `ndk`), attaches default compiler configs (`//gn/skia:default`, `no_exceptions`, `no_rtti`), and selects the active toolchain from [`gn/toolchain/`](toolchain/).
* [`gn/skia.gni`](skia.gni): Declares all `skia_*` feature flags (`declare_args()`) and defines wrapper target templates (`skia_executable`, `skia_source_set`, `skia_static_library`, `skia_shared_library`, `skia_component`) that inject default warning flags and `partition_alloc` dependencies.
* [`gn/skia/BUILD.gn`](skia/BUILD.gn): Defines low-level compiler and linker `config()` blocks (`default`, `optimize`, `warnings`, `debug_symbols`, `extra_flags`, etc.).
* [`gn/toolchain/BUILD.gn`](toolchain/BUILD.gn): Defines `gcc_like`, `msvc`, and `wasm` toolchains and compile/link rules.

### `.gni` Source Lists & Bazel Synchronization
`gn/*.gni` files define variables listing C++ headers and sources consumed by the root [`BUILD.gn`](../BUILD.gn):

* **Auto-Generated `.gni` Files (Do Not Hand-Edit)**:
  Core library source lists—including [`core.gni`](core.gni), [`gpu.gni`](gpu.gni) (Ganesh), [`graphite.gni`](graphite.gni), [`codec.gni`](codec.gni), [`effects.gni`](effects.gni), [`effects_imagefilters.gni`](effects_imagefilters.gni), [`pathops.gni`](pathops.gni), [`pdf.gni`](pdf.gni), [`ports.gni`](ports.gni), [`rust.gni`](rust.gni), [`sksl.gni`](sksl.gni), [`svg.gni`](svg.gni), [`utils.gni`](utils.gni), [`xml.gni`](xml.gni), and [`xps.gni`](xps.gni)—Carry the header `# DO NOT EDIT: This is a generated file.`
  * **Source of Truth**: Their authoritative file lists live in `BUILD.bazel` files under `include/` and `src/`.
  * **How to Update**: After adding or removing core files in `BUILD.bazel`, regenerate the `.gni` files from the repository root:
    ```sh
    make -C bazel generate_gni
    ```
    See [`bazel/exporter_tool/README.md`](../bazel/exporter_tool/README.md) for full details on how Bazel rules map to `.gni` variables.
* **Manually Maintained `.gni` Files**:
  Test, GM, and benchmark lists—such as [`tests.gni`](tests.gni), [`gm.gni`](gm.gni), [`bench.gni`](bench.gni), [`fuzz.gni`](fuzz.gni), and [`sksl_tests.gni`](sksl_tests.gni)—are edited directly (alongside their corresponding `BUILD.bazel` files).

### Helper Scripts (`gn/*.py`)
* [`gn_to_bp.py`](gn_to_bp.py) & [`gn_to_cmake.py`](gn_to_cmake.py): Translates GN build descriptions into Android Soong (`Android.bp`) and CMake files.
* [`compile_sksl_tests.py`](compile_sksl_tests.py) & [`minify_sksl.py`](minify_sksl.py): Invokes `skslc` and `sksl-minify` during GN action steps.

## Common GN Workflows

Before generating build files, ensure dependencies and `./bin/gn` are fetched:
```sh
python3 tools/git-sync-deps
./bin/fetch-gn
```

### Generating Build Directories (`./bin/gn gen`)
```sh
# Standard Debug build with Clang
./bin/gn gen out/Debug --args='cc="clang" cxx="clang++"'

# Release build with Graphite (Metal + Dawn) enabled
./bin/gn gen out/Release --args='
    is_debug=false
    cc="clang"
    cxx="clang++"
    skia_enable_graphite=true
    skia_use_metal=true
    skia_use_dawn=true'

# Inspect all available GN build arguments and their current defaults
./bin/gn args out/Debug --list
```

### Building Targets with Ninja
```sh
ninja -C out/Debug dm viewer nanobench
```
