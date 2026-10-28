---
name: skia/multiply-by-reciprocal-instead-of-dividing-by-constant
description: When scaling by constants in performance-sensitive code or raster pipeline stages, multiply by compile-time reciprocal constants instead of performing runtime division
rubric: safety
severity: nit
---

**Rule.** In performance-sensitive code (such as raster pipeline stages, color
conversion kernels, and hot math loops), multiply by the compile-time reciprocal
of a constant (e.g., `* (1/510.f)` or `* (1.0f / 255.0f)`) rather than dividing
by the constant (`/ 510.f`).

**Why.** Floating-point division instructions have significantly higher latency
and lower throughput than multiplication, causing pipeline stalls in tight loops
and vector execution units. Because floating-point division and multiplication
by reciprocals can produce different rounding, compilers cannot perform this
transformation automatically without fast-math flags.

## Examples

Bad:

```cpp
// Runtime division stalls vector execution units
*r = (cast((rgba      ) & 0x3ff) - 384.f) / 510.f;
*a = (cast((rgba >> 30)        )        ) / 3.f;
float normalized = byte_val / 255.0f;
```

Good:

```cpp
// Compile-time reciprocal multiplication maximizes IPC and throughput
*r = (cast((rgba      ) & 0x3ff) - 384.f) * (1/510.f);
*a = (cast((rgba >> 30)        )        ) * (1/3.f);
float normalized = byte_val * (1.0f / 255.0f);
```
