---
name: skia/use-fixed-width-types-in-sizeof-for-byte-offsets
description: When computing byte offsets, buffer sizes, or memory strides requiring exact bit widths, use fixed-width types like int32_t in sizeof expressions instead of plain int
rubric: readability
severity: nit
---

**Rule.** When calculating byte offsets, buffer sizes, or memory strides where
an exact bit width is required, use explicit fixed-width stdint types (such as
`int32_t` or `uint32_t`) in `sizeof(...)` expressions rather than plain `int` or
`unsigned`. Reserve plain `int` for general integer arithmetic where bit width
is not part of the memory layout contract.

**Why.** In C++, plain `int` has platform-dependent width and is only guaranteed
to be at least 16 bits. Using explicit fixed-width types in `sizeof` expressions
documents the exact byte stride expected by memory layouts, SIMD lanes, or
serialized data, preventing architecture discrepancies and clarifying layout
intent.

## Examples

Bad:

```cpp
// Bad: plain int in sizeof for byte offset calculation
opArg2 = OffsetCtx(ctx.offset + sizeof(int) * N, 1);
```

Good:

```cpp
// Good: explicit fixed-width integer type guarantees exact byte width
opArg2 = OffsetCtx(ctx.offset + sizeof(int32_t) * N, 1);
```
