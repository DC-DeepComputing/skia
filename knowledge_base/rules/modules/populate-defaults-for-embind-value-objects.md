---
name: skia/modules/populate-defaults-for-embind-value-objects
description: When exposing Embind value_object structs with optional fields in CanvasKit TypeScript definitions, populate default values in JavaScript before passing them to C++
rubric: correctness
severity: should_fix
---

**Rule.** When exposing Emscripten Embind `value_object` structs with optional
properties in TypeScript definitions (`index.d.ts`), provide a normalization
helper or constructor in the JavaScript wrapper layer (`.js`) that populates
fallback defaults for all omitted fields before passing the object across the
WASM boundary. Do not pass incoming JavaScript objects directly to C++ bindings
without ensuring all fields mapped by `value_object` are defined.

**Why.** Emscripten Embind's `value_object` binding requires every mapped C++
struct member to be defined on the incoming JavaScript object. If a TypeScript
interface permits fields to be omitted, those properties evaluate to `undefined`
at runtime, causing Embind to throw an error when marshaling the object.

## Examples

Bad:

```cpp
// index.d.ts
export interface SlottableTextProperty {
  text?: string;
  textSize?: number;
  minTextSize?: number;
}

// skottie.js: passing raw JS object directly to Embind without populating defaults
CanvasKit.ManagedAnimation.prototype.setTextSlot = function(key, textValue) {
  return this._setTextSlot(key, textValue);
};
```

Good:

```cpp
// index.d.ts
export interface SlottableTextProperty {
  text?: string;
  textSize?: number;
  minTextSize?: number;
}

// skottie.js: populate default values for all optional fields before passing to Embind
CanvasKit.SlottableTextProperty = function(t) {
  t['text'] = t['text'] || '';
  t['textSize'] = t['textSize'] || 0;
  t['minTextSize'] = t['minTextSize'] || 0;
  return t;
};

CanvasKit.ManagedAnimation.prototype.setTextSlot = function(key, textValue) {
  return this._setTextSlot(key, CanvasKit.SlottableTextProperty(textValue));
};
```
