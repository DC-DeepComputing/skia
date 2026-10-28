---
name: skia/rust-prefer-enum-over-bool-for-binary-alternatives
description: In Rust functions with binary alternatives or modes, pass dedicated enums instead of raw bool parameters to prevent call-site boolean blindness
rubric: readability
severity: nit
---

**Rule.** In Rust functions, prefer passing domain-specific enums or dedicated
two-value enums instead of raw `bool` parameters, especially when multiple
boolean flags are present, when callers already hold a domain enum, or for
binary alternatives (such as byte order `Endianness` or decoding modes). Pair
enums with exhaustive `match` branching rather than boolean `if/else` checks,
avoiding boolean blindness and argument-swapping bugs at call sites.

**Why.** Raw boolean flags cause boolean blindness at call sites (e.g., passing
ambiguous literals like `(false, false)`), risking swapped arguments and hiding
caller intent. Passing dedicated or domain enums self-documents call sites,
avoids decomposing domain states into boolean flags, and leverages
compiler-enforced exhaustive `match` pattern matching.

## Examples

Bad:

```cpp
// Ambiguous binary alternative:
fn read_u16(data: &[u8], off: usize, le: bool) -> Option<u16> { ... }
let count = read_u16(data, ifd_start, false)?;

// Multiple boolean flags decomposing domain states:
fn apply_encoding_factor(
    matrix: &mut ffi::Matrix3x3,
    matrix_bias: &mut [f32; 3],
    pcs_is_xyz: bool,
    is_b2a_tag: bool,
);
convert_to_a2b(&lut_warehouse, false, false);
```

Good:

```cpp
// Dedicated enum with exhaustive match:
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Endianness { Little, Big }
fn read_u16(data: &[u8], off: usize, endian: Endianness) -> Option<u16> { ... }
let count = read_u16(data, ifd_start, Endianness::Big)?;

// Domain enums preserve type semantics at call sites:
enum LutTagType { A2B, B2A }
fn apply_encoding_factor(
    matrix: &mut ffi::Matrix3x3,
    matrix_bias: &mut [f32; 3],
    pcs: moxcms::DataColorSpace,
    tag_type: LutTagType,
);
convert_to_a2b(&lut_warehouse, moxcms::DataColorSpace::Lab, LutTagType::A2B);
```
