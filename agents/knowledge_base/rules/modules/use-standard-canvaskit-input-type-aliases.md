---
name: skia/modules/use-standard-canvaskit-input-type-aliases
description: In CanvasKit TypeScript declarations, use Point or Vector2 for 2D TypedArrays and define named Input* aliases for polymorphic array parameters
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit TypeScript declarations (index.d.ts), use standard
CanvasKit types and aliases for coordinates, vectors, and polymorphic inputs:
- For 2D float values returned or stored as TypedArrays (such as scale, position, or 2D vector properties), type them as Point or Vector2 (= Point); do not use Vector3 (which represents a 3-element number[]).
- For API parameters accepting flexible inputs (such as typed arrays, buffers, or plain number arrays), define and use named Input* aliases (e.g., InputPoint, InputVector2, InputWords, InputLineBreaks) rather than hardcoded primitive typed arrays (like Uint32Array) or ad-hoc unions (like Point | VectorN or Color | Float32Array).

**Why.** In CanvasKit, Point and Vector2 represent 2-element Float32Array
buffers, whereas Vector3 represents a 3-element number[]; typing 2D float
TypedArrays as Vector3 causes runtime dimensionality and array type errors.
Additionally, defining and using named Input* aliases (accepting MallocObj |
TypedArray | number[]) accurately communicates allowable runtime formats, avoids
artificially restricting callers to specific typed arrays, and ensures
consistent marshalling across CanvasKit's WASM bindings.

## Examples

Bad:

```cpp
export interface TransformValue {
    anchor: Point;
    position: Point;
    scale: Vector3;
}

setTransform(key: string, anchor: Point | VectorN, position: Point | VectorN, scale: VectorN,
             rotation: number, skew: number, skew_axis: number): boolean;
buildWithClientInfo(words?: Uint32Array | null,
                    lineBreaks?: Uint32Array | null): Paragraph;
```

Good:

```cpp
export type Vector2 = Point;
export type InputVector2 = InputPoint;
export type InputWords = MallocObj | Uint32Array | number[];
export type InputLineBreaks = MallocObj | Uint32Array | number[];

export interface TransformValue {
    anchor: Point;
    position: Point;
    scale: Vector2;
}

setTransform(key: string, anchor: InputPoint, position: InputPoint, scale: InputVector2,
             rotation: number, skew: number, skew_axis: number): boolean;
buildWithClientInfo(words?: InputWords | null,
                    lineBreaks?: InputLineBreaks | null): Paragraph;
```
