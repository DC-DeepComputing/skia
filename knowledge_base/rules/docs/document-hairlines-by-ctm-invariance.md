---
name: skia/docs/document-hairlines-by-ctm-invariance
description: When documenting hairline stroke semantics (width 0), specify that hairlines do not scale with the CTM rather than defining them as the thinnest stroke
rubric: readability
severity: should_fix
---

**Rule.** When defining or documenting hairline stroke semantics (stroke width
0), specify that hairlines do not scale with the Current Transformation Matrix
(CTM) and remain fixed in device space (typically 1 device pixel), rather than
defining them strictly as the thinnest possible stroke. Stroke widths between 0
and 1 can produce subpixel anti-aliased coverage that appears visually thinner
or fainter than a 1-pixel hairline.

**Why.** In Skia, stroke widths between 0 and 1 render using subpixel
anti-aliased coverage and can visually appear thinner or lower-opacity than an
unscaled 1-pixel hairline. Defining hairlines as 'the thinnest possible line'
misrepresents their behavior; their true defining invariant is CTM invariance
(remaining fixed in device coordinates regardless of canvas scaling).

## Examples

Bad:

```cpp
*   **Hairline**: The thinnest possible line, typically one device pixel wide. A stroke width of 0
    in a paint is interpreted as a hairline.
```

Good:

```cpp
*   **Hairline**: The thinnest visual line, typically one device pixel wide. A stroke width of 0
    in a paint is interpreted as a hairline. Hairlines do not scale with the CTM.
```
