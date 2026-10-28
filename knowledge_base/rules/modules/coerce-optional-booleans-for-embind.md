---
name: skia/modules/coerce-optional-booleans-for-embind
description: When passing optional boolean arguments to Emscripten Embind C++ methods in CanvasKit JavaScript wrappers, explicitly coerce them using double negation (!!param)
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit JavaScript wrapper methods (such as interface.js),
explicitly coerce optional boolean parameters using double-negation (!!param)
before passing them to underlying Emscripten Embind C++ functions (e.g.,
this._method(..., !!param)). Do not forward optional boolean parameters directly
without coercion.

**Why.** In JavaScript, omitted optional arguments evaluate to undefined.
Emscripten Embind bindings expecting a C++ bool cannot safely consume undefined,
which can cause type conversion errors or unintended behavior across the WASM
boundary. Double negation (!!param) ensures omitted values cleanly default to
false and truthy/falsy values coerce to explicit booleans.

## Examples

Bad:

```cpp
CanvasKit['PathBuilder'].prototype['addCircle'] = function(x, y, r, isCCW) {
  // Passing omitted optional argument forwards undefined to C++ Embind binding
  this._addCircle(x, y, r, isCCW);
  return this;
};
```

Good:

```cpp
CanvasKit['PathBuilder'].prototype['addCircle'] = function(x, y, r, isCCW) {
  // Explicitly coerce optional boolean to ensure false is passed across WASM boundary
  this._addCircle(x, y, r, !!isCCW);
  return this;
};
```
