---
name: skia/gn/build-gn-omit-public-headers-from-source-lists
description: When defining GN file lists or Bazel exporter mappings, include private headers in internal source lists and keep public headers exclusively in public lists
rubric: readability
severity: should_fix
---

**Rule.** Include private and internal header files (*.h) in internal GN source
lists (*_sources or *_srcs in *.gni or BUILD.gn) and map both *_srcs and *_hdrs
Bazel filegroups in bazel/exporter_tool/main.go. Conversely, omit public headers
from internal source lists when already declared in corresponding public header
lists (*_public).

**Why.** GN targets combine public and sources for IDE project generators like
Xcode and Visual Studio. Omitting private headers causes IDE project generators
to hide internal headers, while duplicating public headers across both lists
produces duplicate file entries and IDE generation warnings.

## Examples

Bad:

```cpp
// 1. Bazel exporter mapping omitting private headers (*_hdrs):
{Var: "skia_skottie_sources",
 Rules: []string{
     "//modules/skottie/src:core_srcs",
     "//modules/skottie/src/text:text_srcs",
 }},

// 2. GN source list duplicating public header in source list:
skia_encode_rust_png_public = [
  "$_experimental/rust_png/encoder/SkPngRustEncoder.h",
]
skia_encode_rust_png_srcs = [
  "$_experimental/rust_png/encoder/SkPngRustEncoder.cpp",
  "$_experimental/rust_png/encoder/SkPngRustEncoder.h",  # Redundant duplicate of public header
  "$_experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.cpp",
]
```

Good:

```cpp
// 1. Bazel exporter mapping including both sources and private headers:
{Var: "skia_skottie_sources",
 Rules: []string{
     "//modules/skottie/src:core_hdrs",
     "//modules/skottie/src:core_srcs",
     "//modules/skottie/src/text:text_hdrs",
     "//modules/skottie/src/text:text_srcs",
 }},

// 2. GN source list keeping public headers separate and including private headers:
skia_encode_rust_png_public = [
  "$_experimental/rust_png/encoder/SkPngRustEncoder.h",
]
skia_encode_rust_png_srcs = [
  "$_experimental/rust_png/encoder/SkPngRustEncoder.cpp",
  "$_experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.cpp",
  "$_experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h",  # Private header included
]
```
