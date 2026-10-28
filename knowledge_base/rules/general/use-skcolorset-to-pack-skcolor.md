---
name: skia/use-skcolorset-to-pack-skcolor
description: When constructing or asserting SkColor values, use SkColorSetARGB or SkColorSetRGB instead of raw 32-bit hex literals or manual channel bit-shifting
rubric: readability
severity: nit
---

**Rule.** When constructing, packing, or asserting SkColor values, use
SkColorSetARGB or SkColorSetRGB (or SkColorSetA) rather than raw 32-bit hex
literals (e.g., 0xFFBBA570) or manual channel bit shifts (`<< 16`, `<< 8`) and
`0xFF000000` alpha masks. Pass individual channel components explicitly to the
helper.

**Why.** Raw 32-bit hex integers and manual bit shifts obscure individual
channel values, are prone to channel transposition errors across differing
component orders (ARGB vs. RGBA/BGRA), and bypass SkASSERT debug checks that
verify components stay within [0, 255].

## Examples

Bad:

```cpp
// Raw 32-bit hex literal obscures channel breakdown and order
assert_pixel_color(r, pixmap, 0, 0, 0xFFBBA570, "pixel (0,0)");

// Open-coded channel shift with hardcoded opaque alpha mask
return (uint32_t)((val & 0xffff) * (255 / 65535.0f)) << 16 | 0xff000000;
```

Good:

```cpp
// Explicit channel breakdown prevents transposition errors
assert_pixel_color(r, pixmap, 0, 0, SkColorSetARGB(0xFF, 0xBB, 0xA5, 0x70), "pixel (0,0)");

// Idiomatic Skia helper with debug bounds assertions
return SkColorSetRGB(val * (255 / 65535.0f), 0, 0);
```
