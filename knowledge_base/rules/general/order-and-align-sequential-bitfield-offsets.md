---
name: skia/order-and-align-sequential-bitfield-offsets
description: When defining sequential bitfield offsets, use named constants for field widths instead of raw literals, order operands as <prev_offset> + <prev_bits>, and align declarations
rubric: readability
severity: should_fix
---

**Rule.** When defining sequential bitfield offset constants, enum values, or
packed keys, declare named constants (e.g., `kNum<Field>Bits` or `k<Field>Bits`)
for field bit widths rather than using raw numeric literals. Consistently order
addition operands as `<prev_offset> + <prev_bits>` rather than `<prev_bits> +
<prev_offset>`, and maintain vertical columnar alignment across adjacent offset
declarations and assignment operators.

**Why.** Declaring named constants for bit widths documents the allocated field
sizes and prevents desynchronization when bit layouts change or are shared
across backends. Consistently ordering operands as `<prev_offset> + <prev_bits>`
with aligned columns makes packed struct and key layouts predictable, easier to
scan from lowest to highest bit position, and less error-prone when inserting
fields.

## Examples

Bad:

```cpp
// Raw numeric literal instead of named bit-width constant:
static constexpr uint32_t kColorLoadOpOffset = kSampleCountOffset + 3;

// Inconsistent operand ordering (<prev_bits> + <prev_offset>) and misaligned columns:
static constexpr int kColorFormatOffset = kColorNumSamplesOffset + kSampleBits;
static constexpr int kAdditionalFlagOffset = kFormatBits + kColorFormatOffset;
```

Good:

```cpp
// Named constant for bit width and aligned columns:
static constexpr int kNumSampleKeyBits = 3;
static constexpr uint32_t kColorLoadOpOffset = kSampleCountOffset + kNumSampleKeyBits;

// Operands ordered as <prev_offset> + <prev_bits> with aligned columns:
static constexpr int kColorFormatOffset    = kColorNumSamplesOffset + kSampleBits;
static constexpr int kAdditionalFlagOffset = kColorFormatOffset    + kFormatBits;
```
