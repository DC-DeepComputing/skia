---
name: skia/name-transformation-enums-after-source-and-target
description: When defining data transformation enums or conversion helpers, name variants explicitly after both source and destination formats rather than partial operations
rubric: readability
severity: should_fix
---

**Rule.** When defining enum variants, function names, or type aliases for data
transformations and conversions (such as pixel swizzles, row conversions, or
endian swaps), name them explicitly after both the source and target formats
(e.g., `kRgba8ToRgb8`, `kRgba16leToRgba16be`). Do not use vague or partial
operation names (such as `kRgbaToRgb` for transforms that preserve alpha or swap
endianness) that misstate or obscure the transformation's actual behavior.

**Why.** Inaccurate or incomplete transformation names mislead callers about
critical format attributes, such as whether channels are dropped, bit depths
change, or byte order is swapped. Naming both source and destination
specifications makes data conversion paths self-documenting and prevents subtle
data corruption or unexpected channel loss.

## Examples

Bad:

```cpp
// Misleading: used for both 8-bit channel dropping and 16-bit endian swapping
enum ExtraRowTransform {
    kNone_ExtraRowTransform,
    kRgbaToRgb_ExtraRowTransform,
};
```

Good:

```cpp
// Explicitly identifies both source and target format, bit-depth, and endianness:
enum ExtraRowTransform {
    kNone_ExtraRowTransform,
    kRgba8ToRgb8_ExtraRowTransform,
    kRgba16leToRgba16be_ExtraRowTransform,
    kRgba16leToRgb16be_ExtraRowTransform,
};
```
