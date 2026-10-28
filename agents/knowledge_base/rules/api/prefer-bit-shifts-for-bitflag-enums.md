---
name: skia/api/prefer-bit-shifts-for-bitflag-enums
description: When defining bitflag enums or bitmasks, use bit-shift expressions like 1 << N rather than multi-digit binary literals, and use 0 for empty flags
rubric: readability
severity: should_fix
---

**Rule.** When defining bitflag or bitmask enum values, use bit-shift
expressions (`1 << N` or `1u << N`) rather than multi-digit binary literals
(`0b...`), and use plain `0` for empty or none flags (`kNone = 0`). Explicitly
state the bit index to avoid visual miscounting, bit collisions, and maintenance
errors as flags are added.

**Why.** Multi-digit binary literals (such as `0b0010000000`) are visually
difficult to parse and error-prone to count as the number of flags grows,
increasing the risk of accidental bit collisions or miscounted bit positions.
Using bit-shift expressions (`1 << N`) makes the bit index immediately explicit
and simplifies adding or reordering flags.

## Examples

Bad:

```cpp
enum DrawTypeFlags : uint16_t {
    kNone             = 0b0000000000,
    kBitmapText_Mask  = 0b0000000001,
    kBitmapText_LCD   = 0b0000000010,
    kBitmapText_Color = 0b0000000100,
    kNonSimpleShape   = 0b1000000000,
};
```

Good:

```cpp
enum DrawTypeFlags : uint16_t {
    kNone             = 0,
    kBitmapText_Mask  = 1 << 0,
    kBitmapText_LCD   = 1 << 1,
    kBitmapText_Color = 1 << 2,
    kNonSimpleShape   = 1 << 9,
};
```
