---
name: skia/prefer-makeoffset-over-in-place-offset
description: When deriving an offset, inset, or outset SkRect or SkIRect, use makeOffset, makeInset, or makeOutset rather than mutating in place or adjusting edge coordinates inline
rubric: readability
severity: nit
---

**Rule.** When deriving a translated, inset, or outset copy of an SkRect,
SkIRect, or SkRRect, call non-mutating transformation methods like makeOffset(),
makeInset(), or makeOutset() directly. Do not assign or copy the rectangle to a
new variable and mutate it in place with offset(), inset(), or outset(), nor
manually adjust individual edge coordinates (fLeft, fRight, fTop, fBottom)
inline across multiple expressions.

**Why.** Direct copy-transformation methods like makeOffset() and makeInset()
clearly convey geometric intent, enable const initialization, and eliminate
redundant multi-step mutation sequences. Insetting or outsetting upfront also
precomputes adjusted bounds once, avoiding repetitive arithmetic and preventing
accidental sign or edge transposition errors across opposing edges.

## Examples

Bad:

```cpp
// Copying and mutating in place
fReplayClip = clip;
fReplayClip.offset(translation);

// Adjusting individual edge coordinates inline across multiple expressions
auto d0 = sign * (lA*(b.fLeft + tol)  + lB*(b.fTop + tol)    + lC);
auto d1 = sign * (lA*(b.fRight - tol) + lB*(b.fTop + tol)    + lC);
auto d2 = sign * (lA*(b.fRight - tol) + lB*(b.fBottom - tol) + lC);
auto d3 = sign * (lA*(b.fLeft + tol)  + lB*(b.fBottom - tol) + lC);
```

Good:

```cpp
// Constructing the transformed copy directly
fReplayClip = clip.makeOffset(translation);

// Computing the adjusted bounds once using makeInset
SkRect bInset = b.makeInset(tol, tol);
auto d0 = sign * (lA*bInset.fLeft  + lB*bInset.fTop    + lC);
auto d1 = sign * (lA*bInset.fRight + lB*bInset.fTop    + lC);
auto d2 = sign * (lA*bInset.fRight + lB*bInset.fBottom + lC);
auto d3 = sign * (lA*bInset.fLeft  + lB*bInset.fBottom + lC);
```
