---
name: skia/express-padded-bitfield-shifts-as-structured-arithmetic
description: When extracting or packing bitfields with per-channel padding or fixed-stride layouts, express shift amounts as structured arithmetic rather than precalculated constants
rubric: readability
severity: nit
---

**Rule.** When extracting or packing bitfield channels that include per-channel
padding or fixed-stride lane offsets, express shift amounts as structured
arithmetic (such as `(0+6)`, `(16+6)`, `(32+6)`, `(48+6)`) rather than
precalculated sum constants. Decompose the shift into its constituent lane base
offset and intra-channel padding.

**Why.** Writing shift amounts as structured arithmetic explicitly documents the
lane stride and intra-component padding directly in the expression.
Precalculated magic constants obscure the underlying packing layout and make
auditing bit boundaries or catching arithmetic errors significantly harder.

## Examples

Bad:

```cpp
// 10-bit channels stored in 16-bit lanes with 6 bits of low padding
*r = (cast64((_10x6 >>  6) & 0x3ff) - 384.f) * (1/510.f);
*g = (cast64((_10x6 >> 22) & 0x3ff) - 384.f) * (1/510.f);
*b = (cast64((_10x6 >> 38) & 0x3ff) - 384.f) * (1/510.f);
*a = (cast64((_10x6 >> 54) & 0x3ff) - 384.f) * (1/510.f);
```

Good:

```cpp
// 10-bit channels stored in 16-bit lanes with 6 bits of low padding
*r = (cast64((_10x6 >> ( 0+6)) & 0x3ff) - 384.f) * (1/510.f);
*g = (cast64((_10x6 >> (16+6)) & 0x3ff) - 384.f) * (1/510.f);
*b = (cast64((_10x6 >> (32+6)) & 0x3ff) - 384.f) * (1/510.f);
*a = (cast64((_10x6 >> (48+6)) & 0x3ff) - 384.f) * (1/510.f);
```
