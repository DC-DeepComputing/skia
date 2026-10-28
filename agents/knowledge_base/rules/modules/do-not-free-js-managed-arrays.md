---
name: skia/modules/do-not-free-js-managed-arrays
description: In CanvasKit JavaScript bindings, do not call CanvasKit._free(array.byteOffset) on standard JavaScript arrays returned from C++; reserve _free for WASM-heap-backed TypedArrays
rubric: correctness
severity: should_fix
---

**Rule.** In CanvasKit JavaScript bindings, do not call
CanvasKit._free(array.byteOffset) on values returned from C++ as standard
JavaScript arrays (emscripten::val::array() or JSArray). Standard JS arrays are
garbage collected and do not have a byteOffset property; reserve
CanvasKit._free(typedArray.byteOffset) exclusively for TypedArray views backed
by WebAssembly linear heap allocations (such as buffers returned via
emscripten::typed_memory_view).

**Why.** Calling CanvasKit._free(array.byteOffset) on a standard JavaScript
array passes undefined (coerced to a null pointer in WebAssembly), making it an
ineffective no-op based on an erroneous assumption about the return type. While
freeing WASM-backed TypedArray views is required to prevent linear memory leaks,
calling _free on GC-managed JS arrays creates confusion over memory ownership
boundaries.

## Examples

Bad:

```cpp
function Int32ArrayToBidiRegions(int32Array) {
  var ret = [];
  for (var i = 0; i < int32Array.length; i += 3) {
    ret.push({'start': int32Array[i], 'end': int32Array[i+1], 'level': int32Array[i+2]});
  }
  // Bug: int32Array is a standard JS array (JSArray), not a TypedArray; byteOffset is undefined
  CanvasKit._free(int32Array.byteOffset);
  return ret;
}
```

Good:

```cpp
function Int32ArrayToBidiRegions(int32Array) {
  var ret = [];
  for (var i = 0; i < int32Array.length; i += 3) {
    ret.push({'start': int32Array[i], 'end': int32Array[i+1], 'level': int32Array[i+2]});
  }
  // Standard JS arrays are garbage collected and have no WASM linear heap buffer to free
  return ret;
}
```
