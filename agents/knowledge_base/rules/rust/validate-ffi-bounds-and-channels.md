---
name: skia/rust/validate-ffi-bounds-and-channels
description: When translating Rust FFI data into fixed-size C/C++ structs, validate channel counts against corresponding curve containers and verify target array types with static_assert before memcpy
rubric: correctness
severity: blocking
---

**Rule.** When translating parsed or FFI data structures across language
boundaries into fixed-size C/C++ structs (such as skcms_A2B or skcms_B2A),
validate container sizes and channel counts against both source collection
lengths and target struct limits before copying or indexing. Ensure channel
count bounds checks compare against the corresponding stage curve collection
(e.g., input_channels <= input_curves.size()) rather than unrelated collections
like grid_points. When copying fixed-size array members via memcpy, verify
destination buffer types and dimensions with static_assert (e.g., using
std::is_same_v or sizeof) rather than relying on unverified literals or no-op
runtime calculations like std::min(N, N).

**Why.** Fixed-size destination structs and raw memory copies assume exact array
dimensions. Comparing channel counts against unrelated collections (such as grid
points rather than curve arrays) leaves stage data unchecked, risking
out-of-bounds reads or incomplete initialization when copying curves; similarly,
unverified memcpy calls or drifting struct layouts risk silent buffer overruns.

## Examples

Bad:

```cpp
// Comparing channel count against unrelated collection (grid_points instead of curves)
if (rust_a2b.input_channels > rust_a2b.grid_points.size()) {
    return false;
}
out_skcms->input_channels = rust_a2b.input_channels;
for (size_t i = 0; i < rust_a2b.input_curves.size() && i < 3; i++) {
    ToSkcmsCurve(rust_a2b.input_curves[i], &out_skcms->input_curves[i]);
}
size_t grid_pts_size = std::min<size_t>(4, 4);
memcpy(out_skcms->grid_points, rust_a2b.grid_points.data(), grid_pts_size);
```

Good:

```cpp
// Validate channel count against corresponding curve collection and target capacity
if (rust_a2b.input_channels != 3 || rust_a2b.input_curves.size() < 3) {
    return false;
}
out_skcms->input_channels = rust_a2b.input_channels;
for (size_t i = 0; i < 3; i++) {
    ToSkcmsCurve(rust_a2b.input_curves[i], &out_skcms->input_curves[i]);
}
static_assert(std::is_same_v<decltype(out_skcms->grid_points), uint8_t[4]>,
              "skcms_A2B::grid_points must be uint8_t[4]");
if (rust_a2b.grid_points.size() != 4) {
    return false;
}
memcpy(out_skcms->grid_points, rust_a2b.grid_points.data(), 4);
```
