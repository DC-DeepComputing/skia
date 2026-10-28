---
name: skia/prefer-strict-equality-in-javascript
description: When testing equality or optional parameters in JavaScript or TypeScript, use strict equality (=== or !==) and document explicit undefined checks when zero is valid
rubric: readability
severity: should_fix
---

**Rule.** In JavaScript and TypeScript, use strict equality (`===` or `!==`)
instead of loose equality (`==` or `!=`). When testing optional parameters where
zero is a valid value, compare explicitly against `undefined` (`param ===
undefined`) rather than using falsiness checks (`!param`), and document that
zero is valid. When checking exact numeric values like zero, verify with `===
0`.

**Why.** Loose equality (`==` and `!=`) performs implicit type coercion across
different types, where falsy values like empty strings, false, and empty arrays
coerce to zero. Furthermore, truthiness checks (`!param`) incorrectly treat
valid zero values as absent; explicit `=== undefined` checks prevent coercion
bugs, and explaining why zero is valid prevents future refactorings from
inadvertently simplifying to `!param`.

## Examples

Bad:

```cpp
// Loose equality against numeric sentinels or undefined
glyphInfo['dir'] = glyphInfo['dir'] == 0 ? CanvasKit.TextDirection.RTL : CanvasKit.TextDirection.LTR;

if (sc == undefined || st == undefined) {
  // ...
}
```

Good:

```cpp
if (glyphInfo['dir'] === 0) {
  glyphInfo['dir'] = CanvasKit.TextDirection.RTL;
} else {
  glyphInfo['dir'] = CanvasKit.TextDirection.LTR;
}

// Zero is a valid value for sample count or stencil bits.
if (sc === undefined || st === undefined) {
  // ...
}
```
