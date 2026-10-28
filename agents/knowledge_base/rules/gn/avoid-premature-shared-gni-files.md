---
name: skia/gn/avoid-premature-shared-gni-files
description: When sharing a header across multiple GN targets, include the header directly in existing subsystem source lists instead of creating a premature dedicated .gni file
rubric: design
severity: should_fix
---

**Rule.** Do not introduce a new dedicated .gni file solely to share individual
C++ header files across targets. Include the header directly in each consuming
target's existing subsystem source list (*.gni) until a broader shared component
or library target is genuinely required.

**Why.** Unlike C++ implementation files (.cpp/.cc), C++ header files can safely
be listed across multiple GN target source lists without causing duplicate
symbol linker errors or ODR violations. Creating a dedicated .gni file and
import for isolated headers introduces unnecessary build maintenance, extra
imports, and build configuration complexity.

## Examples

Bad:

```cpp
# gn/rust_shared_cpp_sources.gni
rust_shared_cpp_sources = [ "$_rust/SpanUtils.h" ]

# BUILD.gn
import("//gn/rust_shared_cpp_sources.gni")
sources = skia_codec_rust_png + rust_shared_cpp_sources
```

Good:

```cpp
# gn/codec.gni
skia_codec_rust_png = [
  "$_rust/SpanUtils.h",
  "$_src/codec/SkPngRustCodec.cpp",
]

# gn/encode.gni
skia_encode_rust_png = [
  "$_rust/SpanUtils.h",
  "$_src/encode/SkPngRustEncoderImpl.cpp",
]
```
