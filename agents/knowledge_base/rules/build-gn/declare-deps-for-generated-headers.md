---
name: skia/build-gn/declare-deps-for-generated-headers
description: When compiling C++ sources that include generated headers in GN build files, explicitly declare a dependency in deps on the generating target or action
rubric: correctness
severity: should_fix
---

**Rule.** When compiling C++ source files that include generated headers (such
as Rust CXX bridge headers like cxx.h or *.rs.h), ensure the corresponding GN
build target explicitly declares a dependency in deps on the target or action
that produces those headers. Do not rely on implicit header discovery, as clean
or parallel builds can compile C++ translation units before the generated
headers exist on disk.

**Why.** In GN and Ninja, header dependencies are discovered dynamically via
compiler-generated depfiles (.d), which can only be emitted after headers exist
and compilation succeeds. Without an explicit dependency in deps on the
generating action or target, Ninja has no static build graph edge to sequence
header generation before C++ compilation, leading to race conditions and
missing-header build errors during clean or parallel builds.

## Examples

Bad:

```cpp
optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  public_configs = [ ":rust_common_cxx_bridge_public_config" ]
  # Missing explicit dependency on the target generating cxx.h
}
```

Good:

```cpp
optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  public_configs = [ ":rust_common_cxx_bridge_public_config" ]
  deps = [ ":rust_all_ffi_bazel_build" ]  # Action generating cxx.h and bridge headers
}
```
