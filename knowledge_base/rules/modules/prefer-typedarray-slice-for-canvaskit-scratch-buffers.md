---
name: skia/modules/prefer-typedarray-slice-for-canvaskit-scratch-buffers
description: When reading data from CanvasKit scratch WASM buffers in JavaScript, use decoding helpers or scratchObj['toTypedArray']().slice() instead of subscripting numeric pointers or manual HEAP loops
rubric: correctness
severity: blocking
---

**Rule.** In CanvasKit JavaScript bindings, do not index or subscript numeric
scratch pointers (*Ptr), as primitive byte offsets evaluate to undefined in
JavaScript. To extract values written into scratch memory, use dedicated
decoding helpers (such as copyColorFromWasm or copy4x4MatrixFromWasm) or access
the underlying Malloc object via scratchObj['toTypedArray']().slice(...) rather
than directly indexing CanvasKit.HEAP* with manual pointer offset arithmetic and
copy loops.

**Why.** Scratch pointers (*Ptr) are numeric byte offsets into WebAssembly
linear memory, not JavaScript arrays or objects; subscripting them directly
returns undefined, causing comparisons and return values to fail silently.
Furthermore, manual element-by-element loops over CanvasKit.HEAP* require
error-prone byte-to-element offset division, whereas
scratchObj['toTypedArray']().slice() or copy*FromWasm helpers safely extract
copies without boilerplate.

## Examples

Bad:

```cpp
// Bad: subscripting numeric pointer directly returns undefined
this._getColorSlot(key, _scratchColorPtr);
if (_scratchColorPtr[0] == -1) {
  return null;
}
return CanvasKit.Color(_scratchColorPtr[0], _scratchColorPtr[1],
                       _scratchColorPtr[2], _scratchColorPtr[3]);

// Bad: manual HEAP pointer arithmetic and verbose copy loop
this._getVec2Slot(key, _scratchThreeFloatsAPtr);
if (CanvasKit.HEAPF32[_scratchThreeFloatsAPtr/4 + 2] == -1) {
  return null;
}
var twoFloats = new Float32Array(2);
for (var i = 0; i < 2; i++) {
  twoFloats[i] = CanvasKit.HEAPF32[_scratchThreeFloatsAPtr/4 + i];
}
return twoFloats;
```

Good:

```cpp
// Good: use dedicated decoding helper for scratch pointer
this._getColorSlot(key, _scratchColorPtr);
var fourFloats = copyColorFromWasm(_scratchColorPtr);
if (fourFloats[0] == -1) {
  return null;
}
return fourFloats;

// Good: access Malloc TypedArray view and slice
this._getVec2Slot(key, _scratchThreeFloatsAPtr);
var ta = _scratchThreeFloatsA['toTypedArray']();
if (ta[2] === -1) {
  return null;
}
return ta.slice(0, 2);
```
