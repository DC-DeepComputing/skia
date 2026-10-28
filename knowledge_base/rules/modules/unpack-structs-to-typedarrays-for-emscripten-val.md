---
name: skia/modules/unpack-structs-to-typedarrays-for-emscripten-val
description: When serializing C++ structs to JavaScript in CanvasKit Emscripten bindings, unpack coordinates into TypedArrays via MakeTypedArray instead of passing structs directly to val::set
rubric: correctness
severity: blocking
---

**Rule.** In CanvasKit Emscripten bindings, do not pass unregistered C++ structs
(such as SkPoint, SkVector, SkRect, SkV2, or SkColor4f) directly to
emscripten::val::set(). Unpack their components into a local array and convert
them to a JavaScript TypedArray using MakeTypedArray(), or construct an explicit
JSArray or JSObject.

**Why.** Emscripten Embind cannot automatically serialize unregistered C++
structs across the WebAssembly boundary in val::set(), causing runtime binding
errors or silent failures. Unpacking coordinate, vector, or rectangle components
into TypedArrays (such as Float32Array via MakeTypedArray) produces standard,
efficient JavaScript typed arrays that match CanvasKit API conventions.

## Examples

Bad:

```cpp
JSObject trans_val = emscripten::val::object();
trans_val.set("anchor", transform.fAnchorPoint);
trans_val.set("position", transform.fPosition);
trans_val.set("scale", transform.fScale);
```

Good:

```cpp
JSObject trans_val = emscripten::val::object();
const float anchor[] = {transform.fAnchorPoint.fX, transform.fAnchorPoint.fY};
const float position[] = {transform.fPosition.fX, transform.fPosition.fY};
const float scale[] = {transform.fScale.fX, transform.fScale.fY};
trans_val.set("anchor", MakeTypedArray(2, anchor));
trans_val.set("position", MakeTypedArray(2, position));
trans_val.set("scale", MakeTypedArray(2, scale));
```
