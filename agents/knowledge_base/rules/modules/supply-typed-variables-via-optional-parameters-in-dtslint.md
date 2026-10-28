---
name: skia/modules/supply-typed-variables-via-optional-parameters-in-dtslint
description: In CanvasKit dtslint tests, obtain typed test instances via optional test function parameters with early-return guards instead of runtime factory calls
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit TypeScript definition tests (`dtslint`), obtain typed
test variables by declaring them as optional parameters on the test function and
narrowing them with an early-return guard (e.g., `if (!param) return;`). Do not
instantiate or mock objects via runtime factory methods or dummy resource
strings.

**Why.** CanvasKit's `canvaskit-wasm-tests.ts` executes static type checks via
`dtslint` without a runtime WebAssembly environment. Declaring optional
parameters with early returns provides clean, compiler-narrowed dummy types
without requiring mock objects, complex setup, or invalid runtime factory
inputs.

## Examples

Bad:

```cpp
// Constructing instances via runtime factories in dtslint tests
const rt = CK.RuntimeEffect.Make('not real sksl code')!;
const shader = rt.makeShader([])!;
imgf.MakeShader(shader); // $ExpectType ImageFilter
```

Good:

```cpp
// Supplying typed variables via optional parameters and an early-return guard
function imageFilterTests(CK: CanvasKit, colorFilter?: ColorFilter, img?: Image, shader?: Shader) {
    if (!colorFilter || !img || !shader) return;
    imgf.MakeShader(shader); // $ExpectType ImageFilter
}
```
