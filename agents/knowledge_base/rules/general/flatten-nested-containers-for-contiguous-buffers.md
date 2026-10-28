---
name: skia/flatten-nested-containers-for-contiguous-buffers
description: When managing multi-dimensional data or fixed-size contiguous buffers, do not use nested dynamic containers like std::vector<std::vector<T>>; use flattened std::unique_ptr<T[]> with stride arithmetic
rubric: safety
severity: blocking
---

**Rule.** Do not use nested dynamic containers (such as
std::vector<std::vector<T>> or TArray<TArray<T>>) when exposing, copying, or
consuming multi-dimensional data as a contiguous buffer. For fixed-size heap
buffers or multi-dimensional tables that do not require dynamic resizing, prefer
a single contiguous 1D allocation managed by std::unique_ptr<T[]> (or
skia_private::TArray if resizing is required) and index elements using stride
arithmetic (row * stride + col).

**Why.** Nested dynamic containers allocate separate, non-contiguous heap
buffers for each row, causing memory fragmentation, extra pointer indirections,
and buffer overruns if treated as a contiguous block. Furthermore, dynamic
containers like std::vector carry unnecessary member overhead (24 bytes for
pointer, capacity, and size metadata vs. 8 bytes for std::unique_ptr) when
buffer capacity is fixed at allocation.

## Examples

Bad:

```cpp
// Bad: Nested vectors allocate separate non-contiguous heap buffers; data()->data() causes out-of-bounds reads.
// In addition, std::vector carries unnecessary size/capacity member overhead for fixed-size buffers.
std::vector<std::vector<uint8_t>> fGammaTables;

const uint8_t* getGammaTables() const {
    return fGammaTables.empty() ? nullptr : fGammaTables.data()->data();
}
```

Good:

```cpp
// Good: Flatten 2D data into a single contiguous std::unique_ptr<T[]> allocation with minimal overhead.
std::unique_ptr<uint8_t[]> fGammaTables;

const uint8_t* getGammaTables() const {
    return fGammaTables.get();
}

// Access rows via stride arithmetic:
const uint8_t* row = &fGammaTables[row_index * kTableWidth];
```
