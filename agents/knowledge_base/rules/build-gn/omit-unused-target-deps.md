---
name: skia/build-gn/omit-unused-target-deps
description: When configuring target dependencies in GN or Bazel build files, explicitly declare direct deps for included headers, enable layering_check, and omit unused targets
rubric: design
severity: should_fix
---

**Rule.** When defining GN or Bazel build targets (skia_source_set, optional,
skia_cc_library), explicitly declare direct dependencies in deps for all targets
whose headers are included or symbols referenced, rather than relying on
transitive dependencies. In Bazel, enable features = ["layering_check"] to
enforce strict header layering. Do not include unused, redundant, or overly
broad dependencies (such as depending on //:core when only //src/base is
needed), and never depend on sibling implementations to transitively access
shared headers; depend directly on required base or support libraries (e.g.,
//src/base, //src/codec:codec_support_priv).

**Why.** Relying on transitive dependencies makes builds fragile across
refactorings and leads to race conditions during parallel builds when targets
depend on generated or sibling headers. Declaring unused or overly broad
dependencies creates unnecessary build graph edges and slows compilation, while
layering_check ensures header inclusions remain modular and self-contained.

## Examples

Bad:

```cpp
# GN: Missing direct dependencies for included headers
optional("ico_decode_rust") {
  sources = [ "experimental/rust_ico/decoder/impl/SkIcoRustCodec.cpp" ] # includes SkPngRustCodec.h
  deps = [ ":rust_ico_cxx_bridge" ]
}

# Bazel: Missing layering_check and relying on transitive or broad deps
skia_cc_library(
    name = "rust_png",
    srcs = [":srcs"],
    hdrs = [":hdrs"],
    deps = [ "//experimental/rust_png/ffi:cxx_bridge" ],
)
skia_cc_library(
    name = "rust_bmp_codec",
    deps = [ "//:bmp_decode_codec" ], # Sibling dependency for shared headers
)
```

Good:

```cpp
# GN: Direct dependencies explicitly declared in deps
optional("ico_decode_rust") {
  sources = [ "experimental/rust_ico/decoder/impl/SkIcoRustCodec.cpp" ]
  deps = [
    ":rust_ico_cxx_bridge",
    ":bmp_decode_rust",
    ":png_decode_rust",
  ]
}

# Bazel: Direct dependencies declared with layering_check and minimal support deps
skia_cc_library(
    name = "rust_png",
    srcs = [":srcs"],
    hdrs = [":hdrs"],
    features = ["layering_check"],
    deps = [
        "//:core",
        "//experimental/rust_png/ffi:cxx_bridge",
        "//src/base",
    ],
)
skia_cc_library(
    name = "rust_bmp_codec",
    deps = [ "//src/codec:codec_support_priv" ],
)
```
