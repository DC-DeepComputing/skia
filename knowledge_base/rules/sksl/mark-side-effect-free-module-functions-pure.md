---
name: skia/sksl/mark-side-effect-free-module-functions-pure
description: When defining functions without side effects in internal SkSL module files, annotate them with the $pure modifier to enable compiler optimizations
rubric: readability
severity: nit
---

**Rule.** In internal SkSL module files (.sksl), mark functions that do not
produce side effects with the $pure modifier. Do not leave side-effect-free
helper functions or built-in intrinsics unmarked.

**Why.** SkSL assumes function calls produce side effects by default unless
declared with $pure. Annotating side-effect-free internal module functions with
$pure informs compiler analysis (such as HasSideEffects and
IsDynamicallyUniformExpression) and enables optimizations like dead-code
elimination and expression hoisting when the return value is unused.

## Examples

Bad:

```cpp
// In src/sksl/sksl_graphite_frag.sksl
half4 $colorize_grad_buf(float[] arr, int bufferOffset, int numStops, float2 t) {
    ...
}
```

Good:

```cpp
// In src/sksl/sksl_graphite_frag.sksl
$pure half4 $colorize_grad_buf(float[] arr, int bufferOffset, int numStops, float2 t) {
    ...
}
```
