---
name: skia/codec/omit-redundant-result-mapping-checks
description: When converting external or FFI decoder status enums via mapping helpers like MapDecodingResult or ToSkCodecResult, omit redundant special-cased enum checks
rubric: readability
severity: should_fix
---

**Rule.** When translating external or FFI status codes to SkCodec::Result using
a centralized mapping helper (such as MapDecodingResult or ToSkCodecResult), do
not add redundant special-case if checks for individual non-success variants
like IncompleteInput. Delegate all non-success results directly to the helper
function.

**Why.** Centralized mapping helpers already encapsulate complete translation
tables between third-party or Rust decoder error codes and SkCodec::Result.
Manually intercepting specific enum variants before calling the mapping helper
duplicates mapping logic, creates dead branches, and obscures control flow.

## Examples

Bad:

```cpp
rust_bmp::DecodingResult result = fReader->read_image_data();
if (result == rust_bmp::DecodingResult::IncompleteInput) {
    return SkCodec::kIncompleteInput;
}
if (result != rust_bmp::DecodingResult::Success) {
    return MapDecodingResult(result);
}
return SkCodec::kSuccess;
```

Good:

```cpp
rust_bmp::DecodingResult result = fReader->read_image_data();
if (result != rust_bmp::DecodingResult::Success) {
    return MapDecodingResult(result);
}
return SkCodec::kSuccess;
```
