---
name: skia/modules/define-dimension-specific-ts-type-aliases
description: When defining parameters in CanvasKit TypeScript declarations, declare and use dimension-specific type aliases rather than loose VectorN or mismatched vector types
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit TypeScript type definitions (`index.d.ts`), define and
use semantic, dimension-specific type aliases (such as `Vector2` or
`InputVector2`) for parameter and return types rather than loose generic types
(such as `VectorN`) or accepting mismatched dimensions (such as `InputVector3`)
and ignoring unused components.

**Why.** CanvasKit's TypeScript definition file documents and constrains public
APIs for web developers. Using overly permissive types like `VectorN` (which
accepts arrays of any length) or mismatched types like `InputVector3` weakens
compile-time type safety and obscures expected parameter dimensionality in
editor tooltips.

## Examples

Bad:

```cpp
export interface ManagedSkottieAnimation extends SkottieAnimation {
    setTransform(key: string, anchor: Point | VectorN, position: Point | VectorN, scale: VectorN,
                 rotation: number, skew: number, skew_axis: number): boolean;
}
```

Good:

```cpp
export type Vector2 = Point;
export type InputVector2 = InputPoint;

export interface ManagedSkottieAnimation extends SkottieAnimation {
    setTransform(key: string, anchor: InputPoint, position: InputPoint, scale: InputVector2,
                 rotation: number, skew: number, skew_axis: number): boolean;
}
```
