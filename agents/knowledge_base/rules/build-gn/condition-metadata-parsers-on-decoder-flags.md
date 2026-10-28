---
name: skia/build-gn/condition-metadata-parsers-on-decoder-flags
description: When gating metadata or ICC profile parsing targets in build files, condition them only on decoder flags rather than encoder flags
rubric: design
severity: should_fix
---

**Rule.** Do not gate metadata or ICC profile parsing build targets on image
encoder feature flags (such as 'skia_use_*_encode'); condition them strictly on
decoder flags (such as 'skia_use_*_decode'). Decoders require dedicated parser
libraries to validate untrusted input streams, whereas encoders pass through
pre-existing serialized byte buffers directly into output containers without
parsing or deserializing profile data.

**Why.** Coupling metadata parser libraries to encoder feature flags creates
unnecessary build dependencies and pulls unused parser code or FFI bridges into
encoder-only configurations. Restricting parser build targets to decoder flags
maintains clean separation between decode and encode build graphs.

## Examples

Bad:

```cpp
# Bad: Gating ICC profile parsing target on both decode and encode flags
if (skia_use_rust_png_decode || skia_use_rust_png_encode) {
  action("rust_icc_ffi_bazel_build") { ... }
  skia_source_set("rust_icc_cxx_bridge") { ... }
}
```

Good:

```cpp
# Good: Gating ICC profile parser target strictly on decoder flags
if (skia_use_rust_png_decode) {
  action("rust_icc_ffi_bazel_build") { ... }
  skia_source_set("rust_icc_cxx_bridge") { ... }
}
```
