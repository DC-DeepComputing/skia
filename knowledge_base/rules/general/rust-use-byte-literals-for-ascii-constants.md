---
name: skia/rust-use-byte-literals-for-ascii-constants
description: When defining ASCII byte constants, signatures, or character assertions in Rust, use byte character literals (b'x') instead of numeric hex values
rubric: readability
severity: nit
---

**Rule.** In Rust code, use byte character or byte string literals (e.g., `b'B'`
or `b"BM"`) rather than numeric hexadecimal values with inline comments (e.g.,
`0x42 // 'B'`) when defining ASCII constants, file magic signatures, or byte
assertions. For non-ASCII bytes, keep hexadecimal notation (e.g., `0x89`).

**Why.** Byte character literals evaluate directly to `u8` (or `&[u8; N]`) and
are inherently self-documenting, eliminating the need for redundant explanatory
comments while preventing discrepancies between hex codes and commented
character values.

## Examples

Bad:

```cpp
pub const BMP_SIGNATURE: [u8; 2] = [0x42, 0x4D]; // 'B', 'M'

assert_eq!(data[0], 0x42); // 'B'
assert_eq!(data[1], 0x4D); // 'M'
```

Good:

```cpp
pub const BMP_SIGNATURE: [u8; 2] = [b'B', b'M']; // or *b"BM"

assert_eq!(data[0], b'B');
assert_eq!(data[1], b'M');
```
