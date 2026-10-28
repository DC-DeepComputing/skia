---
name: skia/avoid-double-premultiplying-color-components
description: When converting colors using SkPreMultiplyColor or SkPreMultiplyARGB, pass straight unpremultiplied RGB components instead of pre-scaling color channels by alpha
rubric: correctness
severity: should_fix
---

**Rule.** Do not scale RGB color components by alpha prior to passing them into
SkPreMultiplyColor or SkPreMultiplyARGB. Provide straight (unpremultiplied) RGB
inputs so that alpha premultiplication is applied exactly once.

**Why.** SkPreMultiplyColor and SkPreMultiplyARGB explicitly multiply the RGB
channels by alpha. Passing already-scaled RGB components causes double
premultiplication, resulting in incorrectly darkened colors.

## Examples

Bad:

```cpp
int a = 0xFF * (x + y) / (w - 1 + h - 1);
storage[x + y * w] = SkPreMultiplyColor(SkColorSetARGB(a, a * x / (w - 1), a * y / (h - 1), a));
```

Good:

```cpp
const int a = 0xFF * (x + y) / (w - 1 + h - 1),
          r = 0xFF * x / (w - 1),
          g = 0xFF * y / (h - 1),
          b = 0xFF;
storage[x + y * w] = SkPreMultiplyColor(SkColorSetARGB(a, r, g, b));
```
