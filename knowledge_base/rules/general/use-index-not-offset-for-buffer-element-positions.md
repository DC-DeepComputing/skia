---
name: skia/use-index-not-offset-for-buffer-element-positions
description: When naming array or buffer element positions and base positions, use index or baseIndex instead of offset to avoid confusion with domain offsets
rubric: readability
severity: nit
---

**Rule.** When naming variables, parameters, or constants that track element or
slot positions within an array, span, or buffer, name them using 'index' or
'baseIndex' rather than 'offset'. Reserve 'offset' strictly for byte
displacements or domain values (such as gradient stop offsets, character
offsets, or layout coordinates) to avoid conflating buffer subscripts with
domain data.

**Why.** Calling both array element positions and domain values 'offset' creates
ambiguity over whether an identifier represents an element subscript, a byte
displacement, or a domain quantity (such as a normalized float stop offset).
Using 'index' or 'baseIndex' for element positions clearly distinguishes
container subscripts from domain offset values and prevents confusion when
subscripts and domain offsets appear together.

## Examples

Bad:

```cpp
// Ambiguous: 'bufferOffset' and 'colorOffset' are element indices, but 'offset' is a float stop value.
half4 $colorize_grad_buf(float[] arr, int bufferOffset, int numStops, float2 t) {
    int colorOffset = bufferOffset + numStops;
    int lowOffsetIdx = bufferOffset;
    float offset = arr[lowOffsetIdx];
    return ...;
}
```

Good:

```cpp
// Clear: element indices use 'baseIndex' / 'Index', reserving 'offset' for domain float values.
half4 $colorize_grad_buf(float[] colorAndOffsetData,
                         int offsetsBaseIndex,
                         int numStops,
                         float2 t) {
    int colorsBaseIndex = offsetsBaseIndex + numStops;
    int lowOffsetIndex = offsetsBaseIndex;
    float lowOffset = colorAndOffsetData[lowOffsetIndex];
    return ...;
}
```
