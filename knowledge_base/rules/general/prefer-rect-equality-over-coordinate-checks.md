---
name: skia/prefer-rect-equality-over-coordinate-checks
description: When comparing an SkRect against an SkIRect or checking rectangle equality, use SkRect::Make(iRect) == fRect or operator== instead of manual coordinate checks
rubric: readability
severity: nit
---

**Rule.** When checking whether an SkRect matches an SkIRect (such as bounds
rounded out via RoundOut()), compare SkRect::Make(iRect) == fRect rather than
writing out four-way equality checks across fLeft, fTop, fRight, and fBottom.
Likewise, use operator== directly when comparing two rectangles of the same type
instead of manually comparing individual edge coordinates.

**Why.** SkRect and SkIRect provide built-in operator== overloads that compare
all four boundary coordinates. Converting an SkIRect via SkRect::Make and using
operator== is concise, self-documenting, and avoids verbose, error-prone
coordinate-by-coordinate comparisons.

## Examples

Bad:

```cpp
SkIRect srcSubset = RoundOut(srcRect);
if (srcSubset.fLeft == srcRect.fLeft && srcSubset.fTop == srcRect.fTop &&
    srcSubset.fRight == srcRect.fRight && srcSubset.fBottom == srcRect.fBottom) {
    return src;
}
```

Good:

```cpp
SkIRect srcSubset = RoundOut(srcRect);
if (SkRect::Make(srcSubset) == srcRect) {
    return src;
}
```
