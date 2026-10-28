---
name: skia/modules/use-distinct-canvaskit-scratch-pointers-per-call
description: In CanvasKit JavaScript bindings, use distinct scratch memory pointers (such as _scratchFourFloatsAPtr and _scratchFourFloatsBPtr) when marshalling multiple arguments in the same call
rubric: correctness
severity: blocking
---

**Rule.** In CanvasKit JavaScript bindings, do not reuse the same preallocated
scratch memory buffer or pointer for multiple distinct arguments or struct
fields in the same native invocation. Assign separate scratch pointers (e.g.
_scratchFourFloatsAPtr and _scratchFourFloatsBPtr, or _scratchRRectPtr and
_scratchRRect2Ptr) so that subsequent marshalling calls do not overwrite
preceding arguments.

**Why.** CanvasKit uses preallocated scratch buffers in WebAssembly linear
memory to marshal parameters without per-call heap allocations. Reusing the same
scratch buffer across multiple arguments or fields in a single call causes
subsequent copy operations (such as copyRectToWasm after copyColorToWasm) to
overwrite earlier data before the C++ function receives it, resulting in silent
data corruption.

## Examples

Bad:

```cpp
// Reusing _scratchFourFloatsAPtr overwrites strokeColor with boundingBox before the native call
var strokePtr = copyColorToWasm(textValue['strokeColor'], _scratchFourFloatsAPtr);
var boxPtr = copyRectToWasm(textValue['boundingBox'], _scratchFourFloatsAPtr);
this._setTextValue(key, strokePtr, boxPtr);
```

Good:

```cpp
// Use distinct scratch pointers for each parameter in the same call
var strokePtr = copyColorToWasm(textValue['strokeColor'], _scratchFourFloatsAPtr);
var boxPtr = copyRectToWasm(textValue['boundingBox'], _scratchFourFloatsBPtr);
this._setTextValue(key, strokePtr, boxPtr);
```
