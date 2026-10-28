---
name: skia/opts/combine-simd-mask-subwords-bitwise
description: When reducing multi-word SIMD condition masks across split registers, combine sub-word masks bitwise (& for all, | for any) before comparison instead of logical && or ||
rubric: safety
severity: should_fix
---

**Rule.** When testing whether multi-word SIMD masks across split registers
satisfy all-true or any-true conditions, combine mask sub-words using bitwise
operators (& for all, | for any) before performing a single comparison against
the target mask or zero. Do not check each sub-word mask independently using
logical short-circuiting operators (&& or ||).

**Why.** Evaluating mask words independently with logical && or || introduces
redundant comparison instructions and conditional branches with potential branch
mispredictions. Combining sub-words bitwise reduces the evaluation to a single
branchless comparison, minimizing instruction count and register pressure in hot
SIMD kernels.

## Examples

Bad:

```cpp
SI bool all(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] == 0b1111) && (retv[4] == 0b1111);
}
```

Good:

```cpp
SI bool all(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] & retv[4]) == 0b1111;
}
```
