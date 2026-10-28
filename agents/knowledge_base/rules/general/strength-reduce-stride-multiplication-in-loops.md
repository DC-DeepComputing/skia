---
name: skia/strength-reduce-stride-multiplication-in-loops
description: When populating or traversing strided row pointers or buffer offsets in loops, advance pointers incrementally rather than recomputing base + i * stride
rubric: safety
severity: nit
---

**Rule.** When populating an array of row pointers or traversing a strided 2D
buffer in a loop, advance pointers incrementally (e.g., using a running pointer
`ptr += stride` or referencing the prior element `rowPtrs[i] = rowPtrs[i - 1] +
stride`) instead of recomputing `base + i * stride` on each iteration. Hoist the
initial base address before the loop or initialize the first element, and ensure
empty ranges are handled safely.

**Why.** Recomputing `base + i * stride` performs an index multiplication on
every loop iteration, which compilers cannot always strength-reduce if bounds,
strides, or potential overflow cannot be statically determined. Advancing
pointers or row offsets incrementally replaces per-iteration multiplications
with cheap additions in hot pixel and buffer iteration loops.

## Examples

Bad:

```cpp
// 1. Recomputing strided row pointers via indexed multiplication
for (int i = 0; i < numRows; ++i) {
    rowPtrs[i] = static_cast<JSAMPLE*>(plane.writable_addr()) + i * plane.rowBytes();
}

// 2. Strided buffer traversal with per-iteration index multiplication
for (int y = 0; y < height; ++y) {
    const uint8_t* row = base + y * rowBytes;
    processRow(row);
}
```

Good:

```cpp
// 1. Populate strided row pointers using running pointer or previous element
auto row = static_cast<JSAMPLE*>(plane.writable_addr());
for (int i = 0; i < numRows; ++i, row += plane.rowBytes()) {
    rowPtrs[i] = row;
}

// 2. Advance running pointer incrementally in loop header
for (int y = 0; y < height; ++y, base += rowBytes) {
    processRow(base);
}
```
