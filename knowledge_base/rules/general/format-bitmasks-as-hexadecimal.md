---
name: skia/format-bitmasks-as-hexadecimal
description: When formatting bitmasks, flag sets, or packed pixel colors like SkColor into strings, debug logs, or assertion messages, use hexadecimal specifiers instead of decimal
rubric: readability
severity: should_fix
---

**Rule.** When formatting bitmasks, flag values (such as Vulkan `Vk*Flags` or
feature masks), or packed 32-bit pixel colors (such as `SkColor` or `SkPMColor`)
into strings, debug logs, or test failure messages (such as `REPORTER_ASSERT` or
`ERRORF`), use hexadecimal format specifiers (such as `0x%08x`, `0x%x`, or
`%08x`) rather than decimal (`%u` or `%d`).

**Why.** Decimal formatting obscures individual bit flags and packed byte
channels, making active flags and channel differences (alpha, red, green, blue)
difficult to identify. Hexadecimal notation aligns directly with binary bit
layouts, nibbles, and 8-bit color channels, allowing active flags and
per-channel color mismatches to be recognized at a glance.

## Examples

Bad:

```cpp
// Obscures individual bit flags in debug logs or descriptions
SkStringPrintf("formatFeatures: %u", fFormatFeatures);

// Obscures ARGB channel breakdown in test failure messages
REPORTER_ASSERT(reporter, actual == expected, "Color mismatch: %u != %u", actual, expected);
```

Good:

```cpp
// Hexadecimal reveals active bit flags and bitwise combinations
SkStringPrintf("formatFeatures: 0x%08x", fFormatFeatures);

// Hexadecimal clearly separates alpha, red, green, and blue channels
REPORTER_ASSERT(reporter, actual == expected, "Color mismatch: 0x%08x != 0x%08x", actual, expected);
```
