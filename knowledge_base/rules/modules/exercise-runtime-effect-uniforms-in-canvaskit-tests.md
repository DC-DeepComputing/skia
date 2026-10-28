---
name: skia/modules/exercise-runtime-effect-uniforms-in-canvaskit-tests
description: When creating RuntimeEffect shaders in CanvasKit tests, declare uniforms in SkSL and pass values via makeShader rather than hardcoding constants with empty uniform arrays
rubric: testing
severity: should_fix
---

**Rule.** When creating `RuntimeEffect` shaders in CanvasKit tests
(`modules/canvaskit/tests/`), declare uniform variables in the SkSL and pass
values via `rt.makeShader([...])` instead of hardcoding constant values in the
shader and passing an empty uniform array (`[]`).

**Why.** Passing an empty uniform array (`rt.makeShader([])`) bypasses the
uniform marshalling and binding pipeline across the JavaScript/WebAssembly
boundary. Declaring uniforms in SkSL and passing non-empty uniform data
exercises data packing, type conversions, and shader parameter execution
end-to-end.

## Examples

Bad:

```cpp
const rt = CanvasKit.RuntimeEffect.Make(`
half4 main(vec2 fragcoord) {
    return vec4(0.0, 0.0, 1.0, 0.5);
}
`);
const shader = rt.makeShader([]);
```

Good:

```cpp
const rt = CanvasKit.RuntimeEffect.Make(`
uniform float4 color;
half4 main(vec2 fragcoord) {
    return vec4(color);
}
`);
const shader = rt.makeShader([0.0, 0.0, 1.0, 0.5]);
```
