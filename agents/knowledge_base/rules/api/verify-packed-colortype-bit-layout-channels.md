---
name: skia/api/verify-packed-colortype-bit-layout-channels
description: When documenting bit layouts or channel order for packed pixel and color types, verify channel labels match the format name without duplicates
rubric: readability
severity: should_fix
---

**Rule.** When documenting bit layouts or channel order for packed pixel and
color types (such as SkColorType enum entries), verify that channel labels (R,
G, B, A, x) match the format name and little-endian bit ordering without
duplicate or omitted channels. Ensure that multi-channel color formats define
each constituent channel rather than repeating an adjacent channel code.

**Why.** Inaccurate bit-layout comments in public headers mislead callers and
developers implementing format conversions, blitters, or shader unpacking
routines. Duplicate or swapped channel labels can lead to inverted color
channels, incorrect swizzling, or rendering defects.

## Examples

Bad:

```cpp
// Three channel RGB data (10 bits per channel) packed into a LE 32-bit word. The remaining bits
// are ignored and alpha is forced to opaque.
//   Bits: [x:31..30 B:29..20 G:19..10 B:9..0]
kRGB_101010x_SkColorType,
```

Good:

```cpp
// Three channel RGB data (10 bits per channel) packed into a LE 32-bit word. The remaining bits
// are ignored and alpha is forced to opaque.
//   Bits: [x:31..30 B:29..20 G:19..10 R:9..0]
kRGB_101010x_SkColorType,
```
