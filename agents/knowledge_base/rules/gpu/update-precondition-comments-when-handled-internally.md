---
name: skia/gpu/update-precondition-comments-when-handled-internally
description: When implementing alternative draw recording classes in Graphite, update copied doc comments to remove caller preconditions that the implementation handles internally
rubric: readability
severity: nit
---

**Rule.** When implementing alternative or specialized draw recording classes in
Graphite (such as DrawListLayer vs. DrawList), update copied header
documentation comments to remove caller preconditions (such as supplying
compressed painter's order or stencil index in DrawOrder) that the new
implementation satisfies internally. Do not leave stale doc comments that
misstate caller obligations.

**Why.** Copied doc comments that describe caller preconditions for an earlier
implementation are inaccurate and misleading when the new class handles ordering
or state internally. Accurate documentation ensures callers understand the
simplified contract without preparing redundant ordering structures.

## Examples

Bad:

```cpp
// The provided Renderer must be compatible with the 'shape' and 'stroke' parameters. If the
// renderer uses coverage AA, 'ordering' must have a compressed painters order that reflects that.
// If the renderer uses stencil, the 'ordering' must have a valid stencil index as well.
std::pair<DrawParams*, Layer*> recordDraw(...);
```

Good:

```cpp
// The provided Renderer must be compatible with the 'shape' and 'stroke' parameters.
std::pair<DrawParams*, Layer*> recordDraw(...);
```
