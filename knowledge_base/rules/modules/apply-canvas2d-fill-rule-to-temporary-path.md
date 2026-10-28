---
name: skia/modules/apply-canvas2d-fill-rule-to-temporary-path
description: When handling per-operation path settings like fillRule in Canvas2D emulation, apply them to the temporary drawn path rather than mutating the persistent current path
rubric: correctness
severity: should_fix
---

**Rule.** When implementing per-draw operations (such as fill or clip) in HTML
Canvas 2D emulation, apply operation-specific path attributes like fillRule or
FillType to the temporary drawn path or snapshot. Do not mutate the persistent
context path (_currentPath) or set attributes before snapshotting, as
per-operation settings must not alter persistent path state.

**Why.** Under the HTML5 Canvas 2D specification, operation-level arguments like
fillRule only apply to that specific draw call and must not mutate the context's
persistent path. Mutating persistent state causes subsequent draws to leak the
previous operation's fill type, and modifying the path before snapshotting or
drawing an explicit Path2D applies the fill type to the wrong path object.

## Examples

Bad:

```cpp
if (fillRule === 'evenodd') {
  this._currentPath.setFillType(CanvasKit.FillType.EvenOdd);
}
if (!toDraw) {
  toDraw = this._currentPath.snapshot();
}
```

Good:

```cpp
if (!toDraw) {
  toDraw = this._currentPath.snapshot();
}
if (fillRule === 'evenodd') {
  toDraw.setFillType(CanvasKit.FillType.EvenOdd);
}
```
