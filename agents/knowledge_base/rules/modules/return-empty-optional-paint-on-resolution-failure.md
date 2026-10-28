---
name: skia/modules/return-empty-optional-paint-on-resolution-failure
description: When resolving SVG paint servers or IRI references to an optional paint, return std::nullopt or an unengaged optional rather than SK_ColorTRANSPARENT on failure
rubric: correctness
severity: should_fix
---

**Rule.** When resolving an SVG paint, paint server, or IRI reference to a
std::optional<SkPaint> (or SkTLazy<SkPaint>), return an empty or disengaged
optional (std::nullopt or {}) upon resolution failure. Do not return an engaged
optional holding an SkPaint configured with SK_ColorTRANSPARENT.

**Why.** Callers in SVG rendering (such as SkSVGShape and SkSVGText) inspect
optional paint presence via .has_value() or .isValid() to determine whether to
perform a draw call. Returning an engaged paint with transparent color triggers
unnecessary canvas draw operations and potential compositing side effects
instead of honoring SVG 'none' semantics by skipping rendering entirely.

## Examples

Bad:

```cpp
const auto node = this->findNodeById(paint_selector.iri());
if (!node || !node->asPaint(local_ctx, p.get())) {
    p->setColor(SK_ColorTRANSPARENT);
    return p;
}
```

Good:

```cpp
const auto node = this->findNodeById(paint_selector.iri());
if (!node || !node->asPaint(local_ctx, p.get())) {
    return std::nullopt;
}
```
