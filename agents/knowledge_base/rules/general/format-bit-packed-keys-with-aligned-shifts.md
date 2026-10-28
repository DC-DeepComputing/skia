---
name: skia/format-bit-packed-keys-with-aligned-shifts
description: When packing multiple fields into a composite integer key or bitmask, format each component on its own line with vertically aligned shift operators
rubric: readability
severity: nit
---

**Rule.** When packing multiple fields into a composite integer key or bitmask
using bitwise shifts and OR operations, format each field component on its own
line and vertically align the shift operators (`<<`). Do not condense multiple
shifted fields onto fewer lines solely to satisfy column limits.

**Why.** Placing each component on a separate line with vertically aligned shift
operators makes the bit layout, shift offsets, and field components immediately
readable and auditable. Wrapping multiple shifts across arbitrary line breaks
obscures the bitfield structure and increases the risk of bit-overlap bugs.

## Examples

Bad:

```cpp
uint64_t key = static_cast<uint64_t>(width) << 48 | static_cast<uint64_t>(height) << 32 |
               static_cast<uint64_t>(colorType) << 16 | static_cast<uint64_t>(identifier);
```

Good:

```cpp
uint64_t key = static_cast<uint64_t>(width)     << 48 |
               static_cast<uint64_t>(height)    << 32 |
               static_cast<uint64_t>(colorType) << 16 |
               static_cast<uint64_t>(identifier);
```
