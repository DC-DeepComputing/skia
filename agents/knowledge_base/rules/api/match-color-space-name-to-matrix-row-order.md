---
name: skia/api/match-color-space-name-to-matrix-row-order
description: When naming color space enums, conversion helpers, and matrices, order component abbreviations to match transformation matrix rows or plane layout
rubric: readability
severity: should_fix
---

**Rule.** Name color space conversions, enums, plane descriptors, and
transformation matrices so that component abbreviations strictly match the
channel order defined by transformation matrix rows or standard specifications
(e.g., use 'YCgCo' rather than 'YCoCg' when row 1 calculates green difference Cg
and row 2 calculates orange difference Co). Verify that abbreviations in new
identifiers do not transpose or invert the underlying component row sequence.

**Why.** In multiplanar color space representations and matrix transformations
(such as ITU-T H.273), component abbreviations indicate plane indexing and
matrix row order. Transposing channel abbreviations in identifiers (such as
using YCoCg when channel 1 is Cg and channel 2 is Co) creates confusion,
misdocuments plane layouts, and risks callers binding or extracting the wrong
chroma planes.

## Examples

Bad:

```cpp
// Channel 1 computes Cg (-0.25, 0.5, -0.25) and channel 2 computes Co (0.5, 0.0, -0.5), but named YCoCg:
static void make_rgb_to_yuv_matrix_ycocg(float mx[20], int bits, Range range) {
    ...
    float m[20] = {
          0.25f,  0.5f,   0.25f,  0.0f,     addY,
         -0.25f,  0.5f,  -0.25f,  0.0f,    addUV,
           0.5f,  0.0f,  -0.5f,   0.0f,    addUV,
           0.0f,  0.0f,   0.0f,   1.0f,     0.0f,
    };
}
```

Good:

```cpp
// Named YCgCo to match matrix row and plane ordering (Y, Cg, Co per ITU-T H.273):
static void make_rgb_to_yuv_matrix_ycgco(float mx[20], int bits, Range range) {
    ...
    float m[20] = {
          0.25f,  0.5f,   0.25f,  0.0f,     addY,
         -0.25f,  0.5f,  -0.25f,  0.0f,    addUV,
           0.5f,  0.0f,  -0.5f,   0.0f,    addUV,
           0.0f,  0.0f,   0.0f,   1.0f,     0.0f,
    };
}
```
