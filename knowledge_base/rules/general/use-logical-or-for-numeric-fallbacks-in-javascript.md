---
name: skia/use-logical-or-for-numeric-fallbacks-in-javascript
description: When providing default numeric fallback values for optional or nullable properties in JavaScript, use logical OR (|| 0) rather than bitwise OR (| 0)
rubric: correctness
severity: should_fix
---

**Rule.** In JavaScript and TypeScript, use logical OR (`|| 0`) rather than
bitwise OR (`| 0`) when providing a default fallback value of 0 for optional or
nullable array lengths or numeric properties (e.g., `arr && arr.length || 0` or
`(arr && arr.length) || 0`). Do not write `arr && arr.length | 0`.

**Why.** In JavaScript, bitwise OR (`|`) has higher operator precedence than
logical AND (`&&`). Consequently, `arr && arr.length | 0` evaluates as `arr &&
(arr.length | 0)`; if `arr` is null or undefined, the expression short-circuits
and evaluates to `null` or `undefined` instead of defaulting to `0`, passing an
unexpected nullish value to C++/WASM bindings or downstream math. Logical OR
(`||`) has lower precedence than `&&`, ensuring `(arr && arr.length) || 0`
reliably falls back to `0` when the object is falsy.

## Examples

Bad:

```cpp
// Precedence evaluates as: words && (words.length | 0)
// If words is undefined/null, this passes undefined/null rather than 0.
const para = this._buildWithClientInfo(
    bidis, bidiRegions && bidiRegions.length | 0,
    words, words && words.length | 0);
```

Good:

```cpp
// Precedence evaluates as: (words && words.length) || 0
// If words is undefined/null, this correctly defaults to 0.
const para = this._buildWithClientInfo(
    bPtr, bidiRegions && bidiRegions.length || 0,
    wPtr, words && words.length || 0);
```
