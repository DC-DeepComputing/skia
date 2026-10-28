---
name: skia/modules/prefer-jsarray-over-typedarray-for-generic-arrays
description: When writing CanvasKit Emscripten bindings that accept or return generic JavaScript arrays, use JSArray rather than TypedArray aliases like Int32Array
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit Emscripten C++ bindings, use JSArray rather than typed
array aliases (such as Int32Array, Uint8Array, or Float32Array) in function
signatures when accepting or returning a generic JavaScript array
(emscripten::val::array()). Reserve TypedArray aliases specifically for methods
that exchange actual JavaScript TypedArray buffer instances.

**Why.** In CanvasKit's WasmCommon.h, aliases like JSArray, JSObject,
Int32Array, and Float32Array all alias emscripten::val to document the
JavaScript-WASM contract. Using a typed array alias for standard JavaScript
arrays ([]) obscures the return type and falsely implies to readers and binding
consumers that the method produces or expects an underlying typed array buffer.

## Examples

Bad:

```cpp
.function("_getBidiRegions",
          optional_override([](JSString jtext, int dir) -> Int32Array {
              JSArray result = emscripten::val::array();
              // ...
              return result;
          }))
```

Good:

```cpp
.function("_getBidiRegions",
          optional_override([](JSString jtext, int dir) -> JSArray {
              JSArray result = emscripten::val::array();
              // ...
              return result;
          }))
```
