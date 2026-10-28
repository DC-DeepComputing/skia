---
name: skia/modules/use-global-canvaskit-over-ck-in-tests
description: In CanvasKit JavaScript tests, reference CanvasKit APIs through the global CanvasKit object rather than CK to prevent runtime ReferenceErrors
rubric: correctness
severity: blocking
---

**Rule.** In CanvasKit JavaScript unit and GM tests (modules/canvaskit/tests/),
invoke CanvasKit APIs using the global CanvasKit identifier. Do not reference
CK, which is only scoped as a callback parameter in TypeScript type tests
(canvaskit-wasm-tests.ts) and is undefined in the JavaScript test runner
environment.

**Why.** The Karma/Jasmine browser test runner exposes the initialized
WebAssembly module under the global CanvasKit identifier. Referencing CK inside
modules/canvaskit/tests/ causes an uncaught ReferenceError at runtime.

## Examples

Bad:

```cpp
// Inside modules/canvaskit/tests/core_test.js
var surface = CK.MakeOnScreenGLSurface(grContext, 100, 100, null, sample, stencil);
```

Good:

```cpp
// Inside modules/canvaskit/tests/core_test.js
var surface = CanvasKit.MakeOnScreenGLSurface(grContext, 100, 100, CanvasKit.ColorSpace.SRGB, sample, stencil);
```
