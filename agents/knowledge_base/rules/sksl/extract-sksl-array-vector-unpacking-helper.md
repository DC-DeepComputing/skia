---
name: skia/sksl/extract-sksl-array-vector-unpacking-helper
description: When repeatedly assembling vector types from contiguous array or buffer slices in SkSL, extract unpacking logic into a dedicated helper function
rubric: design
severity: nit
---

**Rule.** In SkSL shaders, extract repetitive inline construction of vector
types (such as half4 or float4) from contiguous array or buffer slices into a
dedicated helper function. Call the helper with the array and base offset rather
than manually indexing successive array elements across multiple branches.

**Why.** Manually assembling vectors with sequential index arithmetic
(arr[offset + 0], arr[offset + 1], ...) across multiple branches is repetitive,
prone to off-by-one indexing errors, and clutters call sites. A dedicated
unpacking helper encapsulates stride and offset indexing in a single place.

## Examples

Bad:

```cpp
// In multiple branches of $colorize_grad_buf:
return half4(arr[bufferOffset + 1],
             arr[bufferOffset + 2],
             arr[bufferOffset + 3],
             arr[bufferOffset + 4]);
...
return half4(arr[endBufferIdx - 4],
             arr[endBufferIdx - 3],
             arr[endBufferIdx - 2],
             arr[endBufferIdx - 1]);
```

Good:

```cpp
$pure half4 $half4_from_array(float[] arr, int offset) {
    return half4(arr[offset + 0],
                 arr[offset + 1],
                 arr[offset + 2],
                 arr[offset + 3]);
}

// At call sites:
return $half4_from_array(arr, bufferOffset + 1);
...
return $half4_from_array(arr, endBufferIdx - 4);
```
