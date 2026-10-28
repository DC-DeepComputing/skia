---
name: skia/api/use-decimal-literals-for-scalar-count-enums
description: When defining enum values that represent discrete scalar quantities or counts rather than bit flags, use decimal literals instead of hexadecimal notation
rubric: readability
severity: should_fix
---

**Rule.** When defining enum or enum class values that represent discrete scalar
quantities, counts, or dimensions rather than bit flags, use decimal integer
literals rather than hexadecimal notation (e.g., 1, 2, 4, 8, 16 rather than
0x01, 0x02, 0x04). Reserve hexadecimal literals specifically for bitfields and
bitmasks intended for bitwise combination.

**Why.** Hexadecimal literals in enum definitions strongly imply composable
bitmask flags. Using decimal literals for scalar quantities prevents API
consumers and maintainers from misinterpreting mutually exclusive values as
combinable bit flags.

## Examples

Bad:

```cpp
enum class SampleCount : uint8_t {
    k1  = 0x01,
    k2  = 0x02,
    k4  = 0x04,
    k8  = 0x08,
    k16 = 0x10,
};
```

Good:

```cpp
enum class SampleCount : uint8_t {
    k1  = 1,
    k2  = 2,
    k4  = 4,
    k8  = 8,
    k16 = 16,
};
```
