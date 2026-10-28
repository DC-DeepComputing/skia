---
name: skia/sksl/mark-side-effect-free-sksl-module-functions-pure
description: When defining side-effect-free helper or built-in functions in SkSL modules, mark them with the $pure modifier to enable dead-code elimination
rubric: readability
severity: nit
---

**Rule.** In SkSL module files (such as src/sksl/*.sksl), mark helper, utility,
and built-in function declarations that have no side effects and no out
parameters with the '$pure' modifier.

**Why.** The SkSL compiler queries modifierFlags().isPure() during side-effect
analysis (Analysis::HasSideEffects) and dynamic uniformity evaluation
(Analysis::IsDynamicallyUniformExpression). Marking side-effect-free functions
as $pure allows the compiler to eliminate unused calls, optimize expressions,
and perform dead-code elimination.

## Examples

Bad:

```cpp
half4 $half4_from_array(float[] arr, int offset) {
    return half4(arr[offset + 0],
                 arr[offset + 1],
                 arr[offset + 2],
                 arr[offset + 3]);
}
```

Good:

```cpp
$pure half4 $half4_from_array(float[] arr, int offset) {
    return half4(arr[offset + 0],
                 arr[offset + 1],
                 arr[offset + 2],
                 arr[offset + 3]);
}
```
