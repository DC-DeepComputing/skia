---
name: skia/short-circuit-safemath-loops-on-overflow
description: When accumulating sizes, counts, or offsets with SkSafeMath inside loops performing non-trivial work or allocations, include safe.ok() in the loop condition to short-circuit on overflow
rubric: safety
severity: should_fix
---

**Rule.** When accumulating sizes, counts, or offsets with SkSafeMath inside a
loop that performs non-trivial work, geometry processing, or allocations,
include safe.ok() (or safeMath.ok()) in the loop condition to short-circuit
iteration immediately upon overflow. Do not continue executing iterations once
arithmetic overflow has already invalidated the computation and doomed the
operation.

**Why.** Once SkSafeMath detects an arithmetic overflow, safe.ok() becomes
permanently false and the overall operation will fail and abort. Continuing loop
iteration wastes CPU cycles, geometry processing, and intermediate memory
allocations on operations whose results will immediately be discarded.

## Examples

Bad:

```cpp
SkSafeMath safeMath;
for (int i = 0; i < instanceCount; i++) {
    quadCount = safeMath.addInt(quadCount,
                                gather_lines_and_quads(args.fPath, ...));
}
if (!safeMath.ok()) {
    return;
}
```

Good:

```cpp
SkSafeMath safeMath;
for (int i = 0; i < instanceCount && safeMath.ok(); i++) {
    quadCount = safeMath.addInt(quadCount,
                                gather_lines_and_quads(args.fPath, ...));
}
if (!safeMath.ok()) {
    return;
}
```
