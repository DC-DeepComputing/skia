---
name: skia/document-offset-binary-bias-ranges
description: When packing signed values into unsigned integer or SIMD registers using offset-binary bias, document the bias constant and mapped range rather than only casts
rubric: readability
severity: nit
---

**Rule.** When packing signed values into unsigned integer or SIMD registers
using an offset-binary bias (such as adding 0x80 to map signed values into
unsigned byte lanes for SWAR arithmetic), document the bias addition and the
mapped input/output value ranges in accompanying comments. Do not document only
the intermediate sign-extension or integer casts while leaving the arithmetic
bias and representation invariants unexplained.

**Why.** Offset-binary or biased representations introduce non-obvious
arithmetic offsets and magic constants (such as 0x80 for signed bytes) that
invert sign-bit conventions. Documenting both the bias addition and the mapped
input/output range ensures maintainers understand the encoding invariant and
prevents off-by-one or overflow errors during range adjustments.

## Examples

Bad:

```cpp
// Cast to int8_t to sign extend before casting to uint8_t
windingByte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(fCoarseWinding));
```

Good:

```cpp
// Cast to i8 to sign extend before casting to u8, then apply SWAR bias to map -127/128 to 0/255.
windingByte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(fCoarseWinding));
```
