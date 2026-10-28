---
name: skia/document-nominal-type-properties-in-ts-declarations
description: When adding phantom or branded properties for nominal typing in TypeScript declarations, use a single leading underscore and document their compile-time purpose
rubric: readability
severity: nit
---

**Rule.** When introducing synthetic, phantom, or branded properties to
TypeScript declarations (*.d.ts, *.ts) to enforce nominal typing on opaque or
structurally identical types, name them with a single leading underscore (e.g.,
_type, _brand) rather than Python-style dunder identifiers (e.g., __type__).
Document the property's compile-time purpose in a doc comment and explicitly
state that it does not exist on the underlying object at runtime.

**Why.** TypeScript uses structural typing, so distinct opaque interfaces that
share the same methods or fields are treated as mutually assignable. Adding
phantom or branded properties enforces compile-time nominal differentiation, but
Python-style double underscores are unidiomatic in JavaScript and TypeScript.
Furthermore, without an explanatory doc comment, consumers may assume the
property is populated at runtime, risking confusion and broken property access.

## Examples

Bad:

```cpp
export interface EmbindObject<T extends string> {
    __type__: T;
    delete(): void;
}
```

Good:

```cpp
/**
 * CanvasKit is built with Emscripten and Embind. Embind adds the following methods to all objects
 * that are exposed with it.
 * This _type field is necessary for the TypeScript compiler to differentiate
 * between opaque types such as Shader and ColorFilter. It doesn't exist at runtime.
 */
export interface EmbindObject<T extends string> {
    _type: T;
    delete(): void;
}
```
