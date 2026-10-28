---
name: skia/modules/canvaskit-name-contour-direction-isccw
description: When exposing CanvasKit Path or PathBuilder shape and contour methods, preserve SkPathDirection as an optional isCCW boolean parameter defaulting to clockwise
rubric: correctness
severity: should_fix
---

**Rule.** When exposing SkPath or PathBuilder shape and contour methods (such as
addCircle, addOval, addRect, addRRect, or arcTo) in CanvasKit TypeScript
definitions or Emscripten bindings, preserve the C++ SkPathDirection capability.
Expose contour direction as an optional boolean parameter named isCCW
(defaulting to false / clockwise); do not omit direction, introduce a custom
PathDirection enum, or use non-standard parameter names like dir, direction, or
clockwise.

**Why.** Omitting contour direction when exposing path shape methods loses
feature parity with C++ Skia and prevents callers from controlling path winding
order required for fill rules and path operations. Standardizing direction
across TypeScript declarations and C++ bindings as an optional isCCW boolean
maintains API uniformity and avoids proliferating custom enums.

## Examples

Bad:

```cpp
// index.d.ts: omitting direction or using non-standard names/enums
addCircle(x: number, y: number, r: number): Path;
addRect(rect: InputRect, clockwise?: boolean): PathBuilder;
addOval(oval: InputRect, dir?: PathDirection): PathBuilder;

// canvaskit_bindings.cpp: omitting SkPathDirection
.function("_addCircle", optional_override([](SkPath& self, SkScalar x, SkScalar y, SkScalar r) {
    self.addCircle(x, y, r);
}))
```

Good:

```cpp
// index.d.ts: optional isCCW boolean defaulting to false
addCircle(x: number, y: number, r: number, isCCW?: boolean): Path;
addRect(rect: InputRect, isCCW?: boolean): PathBuilder;
addOval(oval: InputRect, isCCW?: boolean): PathBuilder;

// canvaskit_bindings.cpp: map isCCW to SkPathDirection
.function("_addCircle", optional_override([](SkPath& self, SkScalar x, SkScalar y, SkScalar r, bool ccw) {
    self.addCircle(x, y, r, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
}))
```
