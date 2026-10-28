---
name: skia/sksl/negate-floats-via-sign-bit-xor
description: When synthesizing floating-point negation in code generators or raster pipeline builders, flip the sign bit via bitwise XOR with 0x80000000 instead of multiplying by -1.0
rubric: safety
severity: should_fix
---

**Rule.** In code generators and SIMD/raster pipeline builders, implement
floating-point negation by flipping the IEEE-754 sign bit via bitwise XOR with
0x80000000 rather than multiplying by -1.0f or subtracting from zero.

**Why.** Bitwise XOR has lower instruction latency than floating-point
multiplication on modern CPU and SIMD pipelines, avoids arithmetic pipeline
stalls, and faithfully preserves IEEE-754 sign semantics across special values
such as ±0.0.

## Examples

Bad:

```cpp
fBuilder.push_constant_f(-1.0f, expr.type().slotCount());
fBuilder.binary_op(BuilderOp::mul_n_floats, expr.type().slotCount());
```

Good:

```cpp
// Handle float negation as an integer `x ^ 0x80000000`. This toggles the sign bit.
fBuilder.push_constant_u(0x80000000, expr.type().slotCount());
fBuilder.binary_op(BuilderOp::bitwise_xor_n_ints, expr.type().slotCount());
```
