---
name: skia/core/account-for-operation-order-in-limited-range-ycgco
description: When deriving limited-range RGB-to-YUV transformation matrices per ITU-T H.273, apply range reduction to RGB before matrix transformation for YCgCo, unlike YCbCr
rubric: correctness
severity: should_fix
---

**Rule.** When deriving RGB-to-YUV/YCgCo transformation matrices for
limited-range color spaces per ITU-T H.273, account for the difference in
operation order between color spaces: limited-range YCgCo applies range
reduction to R, G, and B prior to the matrix transformation, whereas YCbCr
applies the matrix transformation before range reduction. Ensure that for
limited-range YCgCo, the scaling and offset terms in the affine matrix properly
incorporate the pre-transformation range reduction.

**Why.** Under ITU-T H.273, converting RGB to limited-range YCgCo scales and
offsets R, G, and B before matrix multiplication, while YCbCr reduces range on
Y, Cb, and Cr afterwards. Applying standard YCbCr transformation order to
limited-range YCgCo calculates incorrect offset and scaling terms in the
resulting affine matrix, corrupting chroma offsets and color conversion.

## Examples

Bad:

```cpp
if (range == Range::kLimited) {
    scaleY = (219 << shift) / denom;
    addY   = ( 16 << shift) / denom;
}
// Incorrect: assumes YCbCr order where range reduction is applied after matrix transformation
float m[20] = {
      0.25f,  0.5f,   0.25f,  0.0f,     addY,
     -0.25f,  0.5f,  -0.25f,  0.0f,    addUV,
       0.5f,  0.0f,  -0.5f,   0.0f,    addUV,
       0.0f,  0.0f,   0.0f,   1.0f,     0.0f,
};
```

Good:

```cpp
if (range == Range::kLimited) {
    scaleY = (219 << shift) / denom;
    addY   = ( 16 << shift) / static_cast<float>(219 << shift);
}
// Correct: accounts for range reduction on RGB before matrix transformation per ITU-T H.273
float m[20] = {
      0.25f,  0.5f,   0.25f,  0.0f,     addY,
     -0.25f,  0.5f,  -0.25f,  0.0f,    chroma05 * scaleY + addY,
       0.5f,  0.0f,  -0.5f,   0.0f,    chroma05 * scaleY + addY,
       0.0f,  0.0f,   0.0f,   1.0f,     0.0f,
};
```
