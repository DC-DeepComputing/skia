---
name: skia/docs/distinguish-coverage-mask-from-alpha-mask
description: When documenting geometry rasterization or SkMaskFilter operations, describe masks as coverage masks rather than alpha masks to distinguish coverage from opacity
rubric: readability
severity: nit
---

**Rule.** In architectural documentation, guides, and comments describing
SkMaskFilter or rasterization pipelines, refer to the geometry mask representing
pixel coverage as a 'coverage mask' rather than an 'alpha mask'. Reserve 'alpha'
or 'opacity' for color channels and blending operations, clarifying that
coverage masks dictate geometric anti-aliasing before shading.

**Why.** While coverage masks are commonly stored in an 8-bit format identical
to an alpha channel, they represent geometric pixel coverage rather than color
transparency. Calling them 'alpha masks' conflates geometry anti-aliasing with
color opacity and obscures SkMaskFilter's role in modifying geometry coverage
before shader evaluation and blending.

## Examples

Bad:

```cpp
- **`SkMaskFilter`**: Operates on the shape's alpha mask, not its color (e.g. blurring the mask before drawing).
```

Good:

```cpp
- **`SkMaskFilter`**: Operates on the shape's coverage mask, not its color (e.g. blurring the mask before drawing).
```
