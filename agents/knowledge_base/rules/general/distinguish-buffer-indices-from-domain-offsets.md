---
name: skia/distinguish-buffer-indices-from-domain-offsets
description: When indexing buffers or arrays in scopes handling domain offsets, use 'Index' for element subscripts rather than overloading 'offset' for both
rubric: readability
severity: should_fix
---

**Rule.** When indexing buffers or arrays in scopes where domain values are also
called 'offset' (such as gradient stop positions or keyframe offsets), use
'Index' (e.g., baseIndex, colorIndex, lowOffsetIndex) for element subscripts and
container positions. Do not overload 'offset' (such as bufferOffset or
colorOffset) for array element indices.

**Why.** In graphics and shading code, 'offset' commonly denotes domain values
such as gradient stop positions, curve parameters, or byte alignments.
Overloading 'offset' to also represent array element positions causes confusion
about whether an identifier holds a domain value or an array subscript,
obscuring data flow and risking logic bugs.

## Examples

Bad:

```cpp
// Ambiguous: 'offset' denotes both array subscripts and gradient stop domain values.
half4 $colorize_grad_buf(float[] arr, int bufferOffset, int numStops, float2 t) {
    int colorOffset = bufferOffset + numStops;
    int lowOffsetIdx = bufferOffset;
    float lowOffset = arr[lowOffsetIdx];
    return lowOffset > t.x ? arr[colorOffset] : ...;
}
```

Good:

```cpp
// Disambiguated: 'Index' denotes array subscripts; 'Offset' denotes domain values.
half4 $colorize_grad_buf(float[] arr, int offsetsBaseIndex, int numStops, float2 t) {
    int colorsBaseIndex = offsetsBaseIndex + numStops * 4;
    int lowOffsetIndex = offsetsBaseIndex;
    float lowOffset = arr[lowOffsetIndex];
    return lowOffset > t.x ? arr[colorsBaseIndex] : ...;
}
```
