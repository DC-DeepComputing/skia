---
name: skia/api/document-parallel-method-rationale
description: When introducing low-level class or virtual methods resembling existing APIs, document their specific purpose, constraints, and why existing entry points cannot be reused
rubric: readability
severity: nit
---

**Rule.** When introducing a low-level class or virtual method that resembles or
appears redundant with an existing entry point (such as an individual vs.
batch/set variant), document its specific purpose, design rationale, and
constraints in header comments. Explicitly explain why existing alternative APIs
cannot be reused for this use case (e.g., parameter processing, constraint
differences, or performance requirements) to prevent future maintainers from
questioning or mistakenly refactoring the method.

**Why.** Without explicit documentation explaining why a similar or higher-level
entry point was not reused, maintainers cannot deduce the technical constraints
that necessitated a separate low-level method. Documenting these trade-offs
preserves architectural context and prevents misguided refactoring or attempts
to consolidate entry points with incompatible requirements.

## Examples

Bad:

```cpp
// Assumes the src and dst rects have already been optimized to fit the proxy.
// Only implemented by the gpu devices.
virtual void drawEdgeAAImage(const SkMatrixProvider&,
                             const SkPaint&,
                             const SkImage*,
                             const SkRect& src,
                             const SkRect& dst,
                             ...);
```

Good:

```cpp
// Assumes the src and dst rects have already been optimized to fit the proxy.
// Only implemented by the gpu devices.
// This method is lowest level draw used for tiled bitmap draws. It doesn't attempt to modify
// its parameters (e.g., adjust src & dst) but just draws the image however it can. It could,
// almost, be replaced with a drawEdgeAAImageSet call for the tiled bitmap draw use case but
// the extra tilemode requirement and the intermediate parameter processing (e.g.,
// trying to alter the SrcRectConstraint) currently block that.
virtual void drawEdgeAAImage(const SkMatrixProvider&,
                             const SkPaint&,
                             const SkImage*,
                             const SkRect& src,
                             const SkRect& dst,
                             ...);
```
