---
name: skia/codec/document-spec-reserved-zero-bits
description: When validating spec-mandated reserved or padding zero bits in bitstream parsers, read into named variables like reserved_zero or add explanatory comments
rubric: readability
severity: nit
---

**Rule.** In binary bitstream and format parsers, document reserved or padding
bits required to be zero per specification rather than performing anonymous
validation checks. Read reserved bits into an explicitly named variable (e.g.,
reserved_zero) before validating against zero, or document the requirement with
an inline specification clause comment.

**Why.** Anonymous checks like flags.readBits(2) == 0 obscure format
specification requirements and make decode failure diagnostics difficult to
trace. Explicitly naming the variable reserved_zero or citing the spec section
clarifies why unused bits are validated and aligns code directly with
specification syntax tables.

## Examples

Bad:

```cpp
gain_curve_num_control_points_minus_1[a] = flags.readBits(5);
gain_curve_use_pchip_slope_flag[a] = flags.readBits(1);
// Anonymous check obscures why these 2 bits must be zero.
RETURN_ON_FALSE(flags.readBits(2) == 0);
```

Good:

```cpp
gain_curve_num_control_points_minus_1[a] = flags.readBits(5);
gain_curve_use_pchip_slope_flag[a] = flags.readBits(1);
// Explicitly named variable documents the spec-mandated reserved field.
const auto reserved_zero = flags.readBits(2);
RETURN_ON_FALSE(reserved_zero == 0);
```
