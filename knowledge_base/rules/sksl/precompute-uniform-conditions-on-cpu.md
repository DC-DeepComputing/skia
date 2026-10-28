---
name: skia/sksl/precompute-uniform-conditions-on-cpu
description: When evaluating invariant conditions, branches, or tolerance checks depending solely on uniforms in GPU shaders, precompute them on the CPU during uniform preparation
rubric: safety
severity: should_fix
---

**Rule.** Do not evaluate invariant conditions, tolerance checks, or branches
inside vertex or fragment shaders when the inputs depend solely on uniform
parameters. Precompute the condition or special case on the CPU during uniform
gathering (e.g., in KeyHelpers or pipeline data gathering) and pass simplified
boolean flags or encode the state directly into uniform values (such as zeroing
out parameters) so the shader branches on trivial comparisons or avoids
branching entirely.

**Why.** Shader stages execute per-vertex or per-fragment across thousands or
millions of invocations. Performing tolerance math, epsilon comparisons, or
invariant branch evaluation inside the shader wastes GPU ALU cycles, increases
register pressure, and causes thread divergence, whereas CPU uniform setup runs
only once per draw or batch.

## Examples

Bad:

```cpp
// Inside fragment shader:
const float SK_ScalarNearlyZero = 1.0 / (1 << 12);
bool onEdge = abs(abs(dRadius) - 1.0) < SK_ScalarNearlyZero;
if (onEdge) {
    ...
}
```

Good:

```cpp
// On CPU during uniform gathering (e.g., KeyHelpers.cpp):
if (std::abs(a) <= SK_ScalarNearlyZero) {
    a = 0.0f; // Encode special case directly into uniform
}
// Inside fragment shader:
if (a == 0.0) {
    ...
}
```
