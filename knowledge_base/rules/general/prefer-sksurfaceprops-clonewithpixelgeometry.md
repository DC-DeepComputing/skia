---
name: skia/prefer-sksurfaceprops-clonewithpixelgeometry
description: When deriving an SkSurfaceProps copy with altered pixel geometry, use cloneWithPixelGeometry instead of unpacking fields into the multi-argument constructor
rubric: readability
severity: nit
---

**Rule.** When deriving an SkSurfaceProps instance with modified pixel geometry
from an existing props instance, call cloneWithPixelGeometry() instead of
manually extracting fields and passing them into the multi-argument
SkSurfaceProps constructor. Retain unchanged properties automatically through
the helper rather than forwarding flags(), textContrast(), and textGamma()
explicitly.

**Why.** Using cloneWithPixelGeometry() is concise, eliminates repetitive getter
boilerplate, and guarantees that existing and future surface properties (such as
text contrast, gamma, or flags) are preserved automatically across copies.

## Examples

Bad:

```cpp
const SkSurfaceProps surfaceProps(this->surfaceProps().flags(),
                                  cinfo.fPixelGeometry,
                                  this->surfaceProps().textContrast(),
                                  this->surfaceProps().textGamma());
```

Good:

```cpp
const SkSurfaceProps surfaceProps =
    this->surfaceProps().cloneWithPixelGeometry(cinfo.fPixelGeometry);
```
