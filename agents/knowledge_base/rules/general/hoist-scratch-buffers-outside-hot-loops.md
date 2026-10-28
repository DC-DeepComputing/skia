---
name: skia/hoist-scratch-buffers-outside-hot-loops
description: When unpacking or staging data across iterations in performance-critical inner loops, declare reusable scratch buffers or arrays outside the loop
rubric: safety
severity: should_fix
---

**Rule.** In performance-critical inner loops (such as SIMD math, rasterization,
or vector flattening loops), declare reusable temporary scratch buffers or
fixed-size staging arrays outside the loop header rather than inside the loop
body. Reuse the outer buffer across iterations instead of re-entering its scope
and re-declaring it inside the loop.

**Why.** Declaring scratch arrays inside tight loops obscures that the buffer is
reused across iterations and risks unnecessary stack frame adjustments or scope
re-entry overhead in unoptimized builds. Hoisting the scratch buffer clarifies
intent and makes storage reuse explicit across hot loop iterations.

## Examples

Bad:

```cpp
for (uint32_t i = 1; i < numSegments; i += 4) {
    skvx::float8 p = skvx::fma(skvx::fma(A, t, B), t, C);
    float out[8];
    p.store(out);
    polyline->appendPoint({out[0], out[1]});
}
```

Good:

```cpp
float out[8];
for (uint32_t i = 1; i < numSegments; i += 4) {
    skvx::float8 p = skvx::fma(skvx::fma(A, t, B), t, C);
    p.store(out);
    polyline->appendPoint({out[0], out[1]});
}
```
