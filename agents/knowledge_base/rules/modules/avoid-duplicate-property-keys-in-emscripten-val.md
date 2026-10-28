---
name: skia/modules/avoid-duplicate-property-keys-in-emscripten-val
description: When populating JavaScript objects or dictionaries using emscripten::val::set in CanvasKit bindings, verify property keys are unique and map to distinct struct members
rubric: correctness
severity: blocking
---

**Rule.** Do not duplicate property keys when populating JavaScript objects or
dictionaries using emscripten::val::set (or similar key-value setters). Verify
that each struct member or dictionary entry maps to its distinct property name
rather than repeating keys from copy-pasted assignment lines.

**Why.** In Emscripten bindings, calling val::set with duplicate keys silently
overwrites the previous property value on the JavaScript object and leaves the
intended property unset. This breaks JavaScript callers, defaults, or
serialization contracts without producing a compile-time warning.

## Examples

Bad:

```cpp
// Duplicate "vertAlign" key overwrites fVAlign and leaves "resize" unset
text_val.set("vertAlign", textProp->fVAlign);
text_val.set("vertAlign", textProp->fResize);
```

Good:

```cpp
text_val.set("vertAlign", textProp->fVAlign);
text_val.set("resize", textProp->fResize);
```
