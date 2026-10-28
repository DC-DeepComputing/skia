---
name: skia/check-loop-break-threshold-before-decrement
description: When implementing countdown loops with early exits in shader or performance-critical code, test the break threshold before decrementing the counter
rubric: safety
severity: nit
---

**Rule.** In shader code (such as SkSL) or performance-critical loops with an
early exit based on a decremented counter, test the exit threshold before
decrementing the counter rather than decrementing first and checking against
zero. Testing before decrementing (e.g., checking if (count <= 1.0) { break; }
before count -= 1.0;) avoids executing dead arithmetic on the terminal
iteration.

**Why.** Decrementing the loop counter before checking an early-exit threshold
causes the final iteration to perform an unnecessary subtraction immediately
before breaking. Testing the threshold beforehand avoids executing dead ALU
operations on the break path, saving GPU cycles in shaders and CPU cycles in
tight loops.

## Examples

Bad:

```cpp
// Bad: executes an unnecessary subtraction on the terminal iteration before breaking
oct -= 1.0;
if (oct <= 0.0) {
    break;
}
```

Good:

```cpp
// Good: tests the threshold before decrementing to avoid dead arithmetic
if (oct <= 1.0) {
    break;
}
oct -= 1.0;
```
