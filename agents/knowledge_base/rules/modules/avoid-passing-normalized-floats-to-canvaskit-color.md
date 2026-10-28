---
name: skia/modules/avoid-passing-normalized-floats-to-canvaskit-color
description: In CanvasKit JavaScript bindings, do not pass normalized [0.0, 1.0] floats into CanvasKit.Color(); return the Float32Array directly or use CanvasKit.Color4f()
rubric: correctness
severity: should_fix
---

**Rule.** In CanvasKit JavaScript wrappers, do not pass normalized [0.0, 1.0]
float color components (such as values returned from copyColorFromWasm()) into
CanvasKit.Color(r, g, b, a). Return the Float32Array directly or construct the
color using CanvasKit.Color4f(r, g, b, a), reserving CanvasKit.Color() strictly
for CSS-style 0–255 integer inputs.

**Why.** CanvasKit.Color() implements the CSS rgba() convention: it expects red,
green, and blue components in the 0–255 range and divides them by 255 to
construct a normalized Float32Array color. Components copied from WebAssembly
via copyColorFromWasm() are already normalized SkColor4f floats in [0.0, 1.0];
passing them to CanvasKit.Color() divides them by 255 a second time, corrupting
channel values down to near zero.

## Examples

Bad:

```cpp
CanvasKit.ManagedAnimation.prototype.getColorSlot = function(key) {
  this._getColorSlot(key, _scratchColorPtr);
  var fourFloats = copyColorFromWasm(_scratchColorPtr);
  if (fourFloats[0] === -1) {
    return null;
  }
  // Bad: CanvasKit.Color divides r, g, b by 255, double-normalizing fourFloats.
  return CanvasKit.Color(fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3]);
};
```

Good:

```cpp
CanvasKit.ManagedAnimation.prototype.getColorSlot = function(key) {
  this._getColorSlot(key, _scratchColorPtr);
  var fourFloats = copyColorFromWasm(_scratchColorPtr);
  if (fourFloats[0] === -1) {
    return null;
  }
  // Good: fourFloats is already a Float32Array of normalized SkColor4f floats.
  return fourFloats;
};
```
