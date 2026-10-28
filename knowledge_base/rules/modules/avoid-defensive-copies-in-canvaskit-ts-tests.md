---
name: skia/modules/avoid-defensive-copies-in-canvaskit-ts-tests
description: In CanvasKit TypeScript definition tests, invoke methods directly on existing shared test instances instead of creating defensive copies or extra intermediate variables
rubric: readability
severity: nit
---

**Rule.** In CanvasKit TypeScript type-definition tests (such as
canvaskit-wasm-tests.ts), invoke tested methods directly on existing shared
instances rather than creating defensive copies (e.g., via .copy()) or declaring
extra intermediate variables. Because type-definition tests are only statically
checked by dtslint and never executed at runtime, mutating shared instances does
not affect subsequent test cases.

**Why.** CanvasKit's canvaskit-wasm-tests.ts suite is evaluated solely by the
TypeScript compiler and dtslint for static type validity without runtime
execution. Creating defensive copies or intermediate variables introduces
unnecessary boilerplate and misleadingly implies that test instances maintain
runtime state across tests.

## Examples

Bad:

```cpp
// Redundantly copying a shared instance to avoid mutating it during type checking
const p8 = p3.copy().addCircle(10, 10, 10);
```

Good:

```cpp
// Invoking tested methods directly on the existing shared instance
builder.addRect(someRect);
builder.addRect(someRect, true);
builder.addCircle(10, 10, 10);
```
