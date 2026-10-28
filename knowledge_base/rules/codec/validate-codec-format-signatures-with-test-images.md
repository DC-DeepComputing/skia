---
name: skia/codec/validate-codec-format-signatures-with-test-images
description: When implementing image codec format sniffers, do not recognize magic bytes or format signatures without dedicated test images verifying decoder support
rubric: testing
severity: should_fix
---

**Rule.** Do not accept magic bytes or format signatures in image codec sniffers
(such as `IsBmp` or `is_bmp_data`) without dedicated test images validating that
the decoder actually supports and handles those format variants. Only recognize
signatures for which corresponding test assets exist and full decoding is
implemented.

**Why.** Claiming support for format signatures or magic byte variants without
test images risks having the codec claim ownership of streams it cannot actually
decode, preventing other decoders from attempting fallback and failing during
subsequent decoding operations. Verifying signatures against actual test images
ensures advertised format support matches true decoder capabilities.

## Examples

Bad:

```cpp
// Claiming support for 'BA' signature without test images validating it:
pub fn is_bmp_data(data: &[u8]) -> bool {
    data.len() >= 2 && data[0] == b'B' && (data[1] == b'M' || data[1] == b'A')
}
```

Good:

```cpp
// Only recognize signatures that have test coverage and decoder support:
// "IC", "PT", "CI", "CP", "BA" are not yet supported by image crate.
pub fn is_bmp_data(data: &[u8]) -> bool {
    data.len() >= 2 && data[0] == b'B' && data[1] == b'M'
}
```
