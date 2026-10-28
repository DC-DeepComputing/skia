---
name: skia/combine-simd-lane-masks-bitwise
description: When combining multiple integer lane masks or flags in SIMD reductions, combine sub-masks bitwise before comparing rather than chaining logical operators
rubric: safety
severity: should_fix
---

**Rule.** In performance-sensitive SIMD vector routines and reduction helpers
(such as any and all), combine multiple integer lane masks or flags using
bitwise operators (| for any, & for all) before comparing against zero or the
target mask, rather than chaining comparisons with logical short-circuit
operators (||, &&).

**Why.** Logical short-circuit operators (||, &&) introduce conditional branches
and multiple test instructions. Combining integer sub-masks or lane flags
bitwise before performing a single comparison enables branchless evaluation and
superior codegen with fewer instructions.

## Examples

Bad:

```cpp
// Bad: Logical operators introduce branching and multiple comparison instructions.
SI bool any(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] != 0b0000) || (retv[4] != 0b0000);
}

SI bool all(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] == 0b1111) && (retv[4] == 0b1111);
}
```

Good:

```cpp
// Good: Combine integer lane masks bitwise before comparing once.
SI bool any(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] | retv[4]) != 0b0000;
}

SI bool all(I32 c) {
    v8i32 retv = (v8i32)__lasx_xvmskltz_w(__lasx_xvslt_wu(__lasx_xvldi(0), c));
    return (retv[0] & retv[4]) == 0b1111;
}
```
