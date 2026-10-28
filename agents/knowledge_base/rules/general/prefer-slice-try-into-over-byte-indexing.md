---
name: skia/prefer-slice-try-into-over-byte-indexing
description: When converting subslices to fixed-size arrays in Rust for initialization or integer decoding, use try_into().unwrap() instead of manual loops or byte-by-byte literals
rubric: readability
severity: nit
---

**Rule.** When converting sub-slices to fixed-size arrays in Rust—whether
initializing a fixed-size array or decoding integers with from_le_bytes or
from_be_bytes—use .try_into().unwrap() or parsing helper functions (such as
byteorder_lite or read_u32). Do not manually initialize a mutable array and copy
elements with a loop, nor enumerate individual bytes into an array literal like
[b[0], b[1], ...].

**Why.** Converting sub-slices to fixed-size arrays with
slice[..N].try_into().unwrap() is idiomatic in Rust, expresses the intended
slice length directly, and eliminates unnecessary mutable state, manual loop
indexing, and error-prone copy-paste element indexing.

## Examples

Bad:

```cpp
// Initializing a fixed-size array:
let mut grid_points = [0u8; 4];
for (i, &gp) in mdt.grid_points.iter().take(4).enumerate() {
    grid_points[i] = gp;
}

// Decoding integers:
let compression = u32::from_le_bytes([data[30], data[31], data[32], data[33]]);
let width = i32::from_le_bytes([data[4], data[5], data[6], data[7]]);
```

Good:

```cpp
// Initializing a fixed-size array:
let grid_points: [u8; 4] = mdt.grid_points[..4].try_into().unwrap();

// Decoding integers:
let compression = u32::from_le_bytes(data[30..34].try_into().unwrap());
let width = i32::from_le_bytes(data[4..8].try_into().unwrap());
// or using dedicated helpers: let count = read_u32(entry, 4, Endianness::Big).unwrap();
```
