---
name: skia/build-gn/export-embedder-sources-to-gni
description: When managing embedder-facing GN source lists, export from Bazel using make generate_gni instead of hardcoding sources in BUILD.gn or manually creating or editing .gni files
rubric: correctness
severity: should_fix
---

**Rule.** Do not hardcode source file paths inline in sources lists of GN build
targets when embedders need them, and never manually create or edit source-list
files in gn/*.gni. Instead, declare filegroup targets in BUILD.bazel, register
export mappings in bazel/exporter_tool/main.go (and custom footers or templates
in bazel/exporter/gni_exporter.go if needed), regenerate GN file lists via make
-C bazel generate_gni, and assign the generated variables in BUILD.gn.

**Why.** Downstream embedders (such as Chromium and Flutter) build Skia by
importing generated gn/*.gni file lists into their own builds rather than
referencing Skia's BUILD.gn directly. Hardcoding sources inline inside BUILD.gn
leaves them inaccessible to embedders, while manually creating or editing .gni
files causes GN and Bazel builds to diverge, failing presubmit verification and
getting overwritten during subsequent export runs.

## Examples

Bad:

```cpp
# BUILD.gn: Hardcoding sources inline prevents embedders from consuming them
skia_source_set("typeface_proxy") {
  sources = [ "src/ports/SkTypeface_proxy.cpp" ]
}

# gn/*.gni: Manually creating or editing source lists bypasses Bazel export
rust_shared_cpp_sources = [ "$_rust/SpanUtils.h" ]
skia_codec_rust_ico = [ "$_experimental/rust_ico/decoder/SkIcoRustDecoder.cpp" ]
```

Good:

```cpp
# 1. Define filegroups in BUILD.bazel (e.g. //rust/common:span_utils)
# 2. Register export mappings in bazel/exporter_tool/main.go:
{Var: "skia_codec_rust_png",
    Rules: []string{
        "//src/codec:rust_png_srcs",
        "//rust/common:span_utils",
    },
},
# 3. Regenerate GN file lists: `make -C bazel generate_gni`
# 4. Reference generated variable in BUILD.gn:
skia_source_set("typeface_proxy") {
  sources = skia_ports_typeface_proxy_sources
}
```
