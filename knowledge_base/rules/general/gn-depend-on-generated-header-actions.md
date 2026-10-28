---
name: skia/gn-depend-on-generated-header-actions
description: When defining GN targets that compile sources including generated headers, explicitly depend on the generating action target via deps or public_deps
rubric: correctness
severity: blocking
---

**Rule.** Ensure GN targets (such as optional(), skia_source_set(), or
source_set()) compiling translation units that include generated headers
explicitly depend on the producing action or target via deps or public_deps. Do
not rely solely on public_configs or include paths without an explicit
dependency edge.

**Why.** Ninja discovers header dependencies dynamically from compiler depfiles
(.d) only after an initial compilation. Without an explicit target dependency on
the generator action, Ninja cannot determine compilation ordering on clean or
parallel builds, leading to race conditions where C++ compilation starts before
generated headers exist.

## Examples

Bad:

```cpp
optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  public_configs = [ ":rust_common_cxx_bridge_public_config" ]
}
```

Good:

```cpp
optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  public_configs = [ ":rust_common_cxx_bridge_public_config" ]
  deps = [ ":rust_all_ffi_bazel_build" ]
}
```
