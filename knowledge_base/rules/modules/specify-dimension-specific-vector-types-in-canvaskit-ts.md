---
name: skia/modules/specify-dimension-specific-vector-types-in-canvaskit-ts
description: In CanvasKit TypeScript declarations, specify concrete dimensioned types like Vector2 or Vector3 instead of arbitrary-dimension VectorN for fixed-size properties
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit TypeScript type declarations (such as types/index.d.ts),
specify concrete dimension-specific types like Vector2, Vector3, or
InputVector2/InputVector3 rather than arbitrary-dimension VectorN when an API
property, parameter, or return value expects a fixed number of components.
Reserve VectorN strictly for generic vector math utilities operating across
arbitrary dimensions.

**Why.** VectorN aliases an arbitrary-length array (number[]), which obscures
the required component count (such as 2D scale or 3D coordinates) and prevents
TypeScript from catching wrong-sized arrays at compile time. Explicit
dimensioned types document coordinate requirements directly in editor tooltips
and enforce compile-time dimensionality checks for API consumers.

## Examples

Bad:

```cpp
export interface TransformValue {
    scale: VectorN;
}

setTransform(key: string, anchor: InputPoint, position: InputPoint, scale: VectorN): boolean;
```

Good:

```cpp
export interface TransformValue {
    scale: Vector2;
}

setTransform(key: string, anchor: InputPoint, position: InputPoint, scale: InputVector2): boolean;
```
