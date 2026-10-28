---
name: skia/assert-bounds-before-indexing-fixed-arrays
description: When populating fixed-capacity arrays or scratch buffers from dynamic counts or element sizes, assert bounds with SkASSERT (against capacity or sizeof) or bail out early
rubric: safety
severity: should_fix
---

**Rule.** When copying dynamic elements or writing to fixed-capacity arrays or
scratch buffers (such as raw C arrays, std::array, or syntax structs), validate
that dynamic element counts or runtime element sizes satisfy maximum capacity
and minimum bounds before indexing, looping, or converting (e.g., SkASSERT(count
<= std::size(buffer)), SkASSERT(elementSize <= sizeof(buffer)), SkASSERT(count
<= kMaxCapacity), or bailing out early if count > std::size(buffer)). Do not
rely solely on assumed maximums or indirect upstream invariants when writing to
fixed-capacity storage. Additionally, assert minimum bounds before decrementing
counts when encoding zero-based or count-minus-one syntax fields to prevent
underflow.

**Why.** Standard fixed-size arrays and scratch buffers do not validate bounds
in their subscript operators or memory operations, risking buffer overflows and
memory corruption if dynamic counts or element formats exceed capacity due to
refactoring or unvalidated input. Referencing std::size(buffer) or
sizeof(buffer) keeps assertions synchronized with array definitions.
Furthermore, unvalidated counts can underflow when calculating count-minus-one
header fields. Validating counts and sizes upfront—via debug assertions or
defensive bailout checks—catches underflow or overflow bugs early.

## Examples

Bad:

```cpp
// Bad: writes into fixed-size scratch buffer without validating count <= std::size(buffer)
double typecastArgs[16];
for (size_t index = 0; index < numSlots; ++index) {
    typecastArgs[index] = *slotVal;
}

// Bad: writes into fixed-size array without verifying count <= capacity
syntax.num_alternate_images = images.size();
for (size_t a = 0; a < images.size(); ++a) {
    syntax.alternate_hdr_headrooms[a] = float_to_uint16(
        alternateHeadrooms[a], 0u, 60000u, 0u, 10000.f);
}

// Bad: relies on scratch buffer sized for expected maximum format without asserting size
uint64_t dstPixel[2] = {};   // expected to accommodate widest config (F32 x 4)
if (!SkConvertPixels(dst, dstPixel, sizeof(dstPixel), src, &color, sizeof(color))) {
    return false;
}

// Bad: decrements count and populates fixed array without asserting kMin and kMax
syntax.gain_curve_num_control_points_minus_1[a] = cubic.fNumControlPoints - 1u;
for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
    syntax.gain_curve_control_points_x[a][c] =
        float_to_uint16(cubic.fX[c], 0u, 64000u, 0u, 1000.f);
}
```

Good:

```cpp
// Good: asserts dynamic count does not exceed std::size(buffer) before writing
double typecastArgs[16];
SkASSERT(numSlots <= std::size(typecastArgs));
for (size_t index = 0; index < numSlots; ++index) {
    typecastArgs[index] = *slotVal;
}

// Good: asserts dynamic count does not exceed fixed array capacity constant
SkASSERT(images.size() <= HeadroomAdaptiveToneMap::kMaxNumAlternateImages);
syntax.num_alternate_images = images.size();
for (size_t a = 0; a < images.size(); ++a) {
    syntax.alternate_hdr_headrooms[a] = float_to_uint16(
        alternateHeadrooms[a], 0u, 60000u, 0u, 10000.f);
}

// Good: asserts dynamic element size does not exceed fixed buffer capacity
uint64_t dstPixel[2] = {};   // large enough for widest config (F32 x 4)
SkASSERT((size_t)dst.bytesPerPixel() <= sizeof(dstPixel));
if (!SkConvertPixels(dst, dstPixel, sizeof(dstPixel), src, &color, sizeof(color))) {
    return false;
}

// Good: asserts both minimum and maximum bounds before decrementing and indexing
SkASSERT(PiecewiseCubicFunction::kMinNumControlPoints <= cubic.fNumControlPoints);
SkASSERT(cubic.fNumControlPoints <= PiecewiseCubicFunction::kMaxNumControlPoints);
syntax.gain_curve_num_control_points_minus_1[a] = cubic.fNumControlPoints - 1u;
for (uint8_t c = 0; c < cubic.fNumControlPoints; ++c) {
    syntax.gain_curve_control_points_x[a][c] =
        float_to_uint16(cubic.fX[c], 0u, 64000u, 0u, 1000.f);
}
```
