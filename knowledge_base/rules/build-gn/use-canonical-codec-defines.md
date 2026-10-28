---
name: skia/build-gn/use-canonical-codec-defines
description: When configuring image codec targets in build files, export canonical SK_CODEC_DECODES_<FORMAT> or SK_CODEC_ENCODES_<FORMAT> macros and avoid redundant secondary defines like SK_USE_*
rubric: readability
severity: should_fix
---

**Rule.** In build configurations (such as BUILD.gn or Bazel), export only
canonical SK_CODEC_DECODES_<FORMAT>[_WITH_<BACKEND>] and
SK_CODEC_ENCODES_<FORMAT>[_WITH_<BACKEND>] preprocessor defines on codec
targets. Do not introduce ad-hoc or redundant secondary feature defines (such as
SK_USE_<BACKEND>_<FORMAT>_DECODE or SK_USE_*).

**Why.** Codec targets and consumers across Skia consistently standardize on the
SK_CODEC_DECODES_* and SK_CODEC_ENCODES_* macro family for conditional
compilation. Defining redundant secondary macros fragments feature detection,
creates preprocessor clutter, and introduces inconsistent gating across build
files and platforms.

## Examples

Bad:

```cpp
optional("bmp_decode_rust") {
  enabled = skia_use_rust_bmp_decode
  public_defines = [
    "SK_CODEC_DECODES_BMP_WITH_RUST",
    "SK_USE_RUST_BMP_DECODE",
  ]
}
```

Good:

```cpp
optional("bmp_decode_rust") {
  enabled = skia_use_rust_bmp_decode
  public_defines = [
    "SK_CODEC_DECODES_BMP_WITH_RUST",
  ]
}
```
