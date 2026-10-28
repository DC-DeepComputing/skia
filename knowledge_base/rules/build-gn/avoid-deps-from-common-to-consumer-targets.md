---
name: skia/build-gn/avoid-deps-from-common-to-consumer-targets
description: When shared or common GN targets need header include paths, extract a shared config with include_dirs instead of depending on consumer targets in deps
rubric: design
severity: should_fix
---

**Rule.** Do not add build dependencies (`deps`) from shared or common utility
targets to specific consumer or feature modules solely to obtain header include
search paths. Extract a shared `config` specifying the required `include_dirs`
and reference it via `public_configs` or `configs` on both targets to prevent
inverted dependency layering.

**Why.** Declaring a dependency from a shared utility target to a downstream
consumer target inverts dependency layering, risks circular dependencies, and
couples common code to feature-specific build artifacts. Defining a shared
config with include_dirs provides necessary header search paths without creating
unwanted target dependency edges.

## Examples

Bad:

```cpp
# Common utility target depends on downstream consumer module solely for include paths
optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  deps = [ ":rust_png_cxx_bridge" ]
}
```

Good:

```cpp
# Extract shared include_dirs into a config referenced via public_configs
config("rust_common_cxx_bridge_public_config") {
  include_dirs = [ "$root_out_dir" ]
}

optional("rust_common") {
  enabled = skia_use_rust_png_decode || skia_use_rust_png_encode
  public = skia_rust_common_ffi_cpp_hdrs
  sources = skia_rust_common_ffi_cpp_srcs
  public_configs = [ ":rust_common_cxx_bridge_public_config" ]
}
```
