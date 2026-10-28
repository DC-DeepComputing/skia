---
name: skia/document-directional-parameter-roles-by-mode
description: When declaring or implementing helper functions with directional parameters whose roles invert across caller modes, document what each parameter represents for each mode
rubric: readability
severity: nit
---

**Rule.** When declaring or implementing helper functions with directional
parameters (such as `src`/`dst` or `input`/`output`) whose roles invert or vary
across caller modes (such as backdrop vs. regular filtering), document what each
parameter represents in each mode in the function's doc comment. Explicitly
clarify the conceptual mapping so callers and maintainers understand dataflow,
coordinate spaces, and clipping expectations.

**Why.** When directional parameters swap conceptual roles depending on mode,
readers cannot rely on intuitive source-to-destination naming alone. Documenting
parameter semantics across all modes prevents confusion when reasoning about
coordinate transformations, clips, and color spaces, reducing bugs during
refactoring and call-site updates.

## Examples

Bad:

```cpp
// The dst is always required, the src can be null if 'filter' is non-null and does not require
// a source image.
void SkCanvas::internalDrawDeviceWithFilter(SkDevice* src, SkDevice* dst, ...);
```

Good:

```cpp
// The dst is always required, the src can be null if 'filter' is non-null and does not require
// a source image. For regular filters, 'src' is the layer and 'dst' is the parent device. For
// backdrop filters, 'src' is the parent device and 'dst' is the layer.
void SkCanvas::internalDrawDeviceWithFilter(SkDevice* src, SkDevice* dst, ...);
```
