---
name: skia/avoid-var-in-javascript
description: When declaring variables in JavaScript, TypeScript, or HTML scripts, avoid var and prefer const over let for bindings that are not reassigned
rubric: readability
severity: nit
---

**Rule.** In JavaScript, TypeScript, and HTML script blocks, do not use `var`
for variable declarations; declare variables using `const` by default for
bindings that are never reassigned after initialization, and reserve `let` only
for loop counters or variables whose values are mutated.

**Why.** `var` is function-scoped and hoisted, risking variable leakage and
accidental shadowing. Using block-scoped `const` by default communicates
immutability, prevents accidental reassignment bugs, and clarifies data flow,
while reserving `let` specifically for mutable state.

## Examples

Bad:

```cpp
var height = 40;
var bidi = CanvasKit.Bidi.Make();
let rejectWithNoClip = canvas.quickReject(rect);
for (var i = 0; i < regions.length; ++i) {
    var result = '[' + region.start + ':' + region.end + '): ';
}
```

Good:

```cpp
let height = 40; // mutated later
const bidi = CanvasKit.Bidi.Make();
const rejectWithNoClip = canvas.quickReject(rect);
for (let i = 0; i < regions.length; ++i) {
    const result = '[' + region.start + ':' + region.end + '): ';
}
```
