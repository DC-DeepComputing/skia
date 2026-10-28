---
name: skia/modules/free-temporary-canvaskit-wasm-arrays
description: When marshalling temporary arrays across CanvasKit JavaScript bindings, pass pre-allocated scratch pointers to copy1dArray or clean up allocations with freeArraysThatAreNotMallocedByUsers
rubric: safety
severity: should_fix
---

**Rule.** In CanvasKit JavaScript bindings, avoid WASM heap memory leaks when
marshalling temporary array arguments across the FFI boundary. For fixed-size
temporary arrays (such as matrices or vectors), pass an existing pre-allocated
scratch buffer pointer (e.g., _scratch3x3MatrixPtr or _scratchFourFloatsAPtr) as
the destination pointer to copy1dArray. For variable-sized arrays allocated
without a scratch buffer, clean up the temporary WASM heap memory immediately
after the native call using freeArraysThatAreNotMallocedByUsers(ptr,
originalArray); do not assume C++ Skia APIs taking SkSpan or raw pointers take
ownership.

**Why.** Calling copy1dArray without a destination pointer allocates new WASM
linear heap memory via CanvasKit._malloc. Because C++ Skia APIs copy input
buffers rather than taking ownership, omitting
freeArraysThatAreNotMallocedByUsers leaks memory on every invocation. Using
pre-allocated scratch buffers avoids WASM allocator churn and eliminates leaks
without manual deallocation.

## Examples

Bad:

```cpp
// Variant 1: Omitting scratch pointer allocates unmanaged WASM memory that leaks
const tPtr = CanvasKit.copy1dArray(transformData, 'HEAPF32');
this._setTransform(key, tPtr);

// Variant 2: Leaving dynamically allocated WASM heap buffers uncollected
var verbsPtr = copy1dArray(verbs, 'HEAPU8');
var pointsPtr = copy1dArray(pts, 'HEAPF32');
return CanvasKit.Path._MakeFromVerbsPointsWeights(
    verbsPtr, verbs.length, pointsPtr, pts.length / 2);
```

Good:

```cpp
// Variant 1: Pass pre-allocated scratch buffer to avoid heap allocation
const tPtr = copy1dArray(transformData, 'HEAPF32', _scratch3x3MatrixPtr);
this._setTransform(key, tPtr);

// Variant 2: Clean up dynamically allocated buffers after native call
var verbsPtr = copy1dArray(verbs, 'HEAPU8');
var pointsPtr = copy1dArray(pts, 'HEAPF32');
var path = CanvasKit.Path._MakeFromVerbsPointsWeights(
    verbsPtr, verbs.length, pointsPtr, pts.length / 2);
freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
freeArraysThatAreNotMallocedByUsers(pointsPtr, pts);
return path;
```
