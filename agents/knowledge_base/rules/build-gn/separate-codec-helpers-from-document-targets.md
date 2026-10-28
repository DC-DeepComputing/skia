---
name: skia/build-gn/separate-codec-helpers-from-document-targets
description: When configuring document or serializer targets in BUILD.gn or Bazel, keep primary targets unbundled and expose optional codecs as additive helper targets instead of inverted `_no_<feature>` targets
rubric: design
severity: should_fix
---

**Rule.** Do not bundle optional image codec dependencies (such as libpng, Rust
PNG, or JPEG) or adapter headers directly into core document or serializer build
targets (such as 'pdf', 'xps', or 'pdf_writer'), and do not offer inverted
`_no_<feature>` targets (such as 'pdf_writer_no_jpeg'). Keep primary library
targets unbundled and factor codec integrations into separate additive helper
targets (such as 'pdf_jpeg_helpers' or 'xps_libpng_helpers') that consumers can
compose into their dependencies.

**Why.** Skia decouples document generators (e.g., PDF, XPS) and canvas writers
from concrete image codecs via dependency injection. Bundling optional codecs
into primary targets or exposing inverted `_no_<feature>` targets creates
mutually exclusive targets in build dependency DAGs, breaks modular layering,
and forces unwanted transitive dependencies onto consumers who inject custom
codecs.

## Examples

Bad:

```cpp
# GN: Bundling codec dependencies into core document target
optional("xps") {
  enabled = skia_use_xps && is_win
  public = skia_xps_public
  if (skia_use_libpng_encode) {
    public += skia_xps_libpng_public
  }
}

# Bazel: Bundling into default alias and offering an inverted target
alias(
    name = "pdf_writer",
    actual = "//src/pdf:pdf_and_jpeg",
)
alias(
    name = "pdf_writer_no_jpeg",
    actual = "//src/pdf:pdf",
)
```

Good:

```cpp
# GN: Decoupled target and additive helper target
optional("xps") {
  enabled = skia_use_xps && is_win
  public = skia_xps_public
}
optional("xps_libpng_helpers") {
  enabled = skia_use_libpng_encode
  deps = [ ":png_encode_libpng" ]
  public = skia_xps_libpng_public
}

# Bazel: Unbundled default target and additive helper alias
alias(
    name = "pdf_writer",
    actual = "//src/pdf:pdf",
)
alias(
    name = "pdf_jpeg_helpers",
    actual = "//src/pdf:jpeg_helpers",
)
```
