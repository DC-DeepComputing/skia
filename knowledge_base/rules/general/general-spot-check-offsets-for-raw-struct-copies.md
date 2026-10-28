---
name: skia/general-spot-check-offsets-for-raw-struct-copies
description: When copying structs via memcpy or relying on memory layouts across FFI boundaries, assert standard layout, trivial copyability, and spot-check member offsets
rubric: safety
severity: should_fix
---

**Rule.** When copying struct data via memcpy or relying on exact memory layouts
across FFI, serialization, or binary parser boundaries, do not rely solely on
sizeof and alignof. In addition, assert std::is_standard_layout_v and
std::is_trivially_copyable_v, and spot-check critical or non-obvious member
offsets using static_assert with offsetof.

**Why.** memcpy is only defined for trivially copyable types, and raw memory
copying between structs assumes standard layout and matching field offsets.
Relying solely on sizeof and alignof will not catch non-trivial members, virtual
dispatch tables, padding discrepancies, or reordered fields, causing silent
memory corruption or undefined behavior.

## Examples

Bad:

```cpp
static_assert(sizeof(Matrix3x3) == sizeof(skcms_Matrix3x3),
              "Matrix3x3 size mismatch");
static_assert(alignof(Matrix3x3) == alignof(skcms_Matrix3x3),
              "Matrix3x3 alignment mismatch");
memcpy(out_skcms, &rust_matrix, sizeof(skcms_Matrix3x3));
```

Good:

```cpp
static_assert(sizeof(Matrix3x3) == sizeof(skcms_Matrix3x3),
              "Matrix3x3 size must match skcms_Matrix3x3");
static_assert(alignof(Matrix3x3) == alignof(skcms_Matrix3x3),
              "Matrix3x3 alignment must match skcms_Matrix3x3");
static_assert(std::is_standard_layout_v<Matrix3x3> &&
              std::is_trivially_copyable_v<Matrix3x3>,
              "Matrix3x3 must be standard layout and trivially copyable");
static_assert(offsetof(skcms_Matrix3x3, vals) == 0,
              "skcms_Matrix3x3::vals must be at offset 0");
memcpy(out_skcms, &rust_matrix, sizeof(skcms_Matrix3x3));
```
