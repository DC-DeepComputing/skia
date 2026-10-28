---
name: skia/prefer-skspan-const-for-read-only-data
description: When declaring, slicing, or passing spans for read-only buffer data, use SkSpan<const T> rather than mutable SkSpan<T>, and omit redundant top-level const
rubric: readability
severity: should_fix
---

**Rule.** When declaring, slicing, or passing spans for read-only buffer data,
specify const on the element type (SkSpan<const T> or std::span<const T>) rather
than mutable SkSpan<T>. In virtual interfaces and abstract base classes, keep
read-only buffer parameters const even if a derived backend calls a legacy C API
requiring non-const pointers; isolate any necessary const_cast to that
implementation boundary. Avoid top-level const on pass-by-value span parameters
(such as const SkSpan<T> or const SkSpan<const T>), as it adds visual noise
without altering call semantics and fails to enforce element immutability when
the element type itself is non-const.

**Why.** Like raw pointers, spans have shallow constness: a const SkSpan<T>
prevents reassigning the span variable itself but still allows mutating
referenced buffer elements. Qualifying element types as const enforces read-only
access at compile time and allows callers to pass const buffers without casting.
Furthermore, because spans are lightweight view types designed to pass by value,
top-level const on parameter objects provides no caller-side safety and adds
unnecessary visual noise. Isolating unavoidable C-library const-casts within
specific backend implementations preserves the const contract across the public
interface and sibling implementations.

## Examples

Bad:

```cpp
// Bad: Non-const element type allows mutation of read-only source buffer
SkSpan<uint8_t> srcRow = fullImageBuffer.subspan(offset, rowBytes);
this->applyFilter(dstRow, srcRow);

// Bad: Top-level const on SkSpan has shallow constness; elements remain mutable
const SkSpan<uint8_t> inputSpan = buffer.first(chunkSize);

// Bad: Redundant top-level const on pass-by-value span parameter
static VertexFiller Make(const SkSpan<const SkPoint> positions);

// Bad: Weakens virtual interface contract because one C backend requires non-const
class SkPngEncoderBase {
    virtual bool onEncodeRow(SkSpan<uint8_t> row) = 0;
};
```

Good:

```cpp
// Good: Enforces compile-time read-only access to buffer elements
SkSpan<const uint8_t> srcRow = fullImageBuffer.subspan(offset, rowBytes);
this->applyFilter(dstRow, srcRow);

// Good: Read-only span parameter and view use const element type without top-level const
void processChunk(SkSpan<const uint8_t> inputSpan, SkSpan<uint8_t> outputSpan);
static VertexFiller Make(SkSpan<const SkPoint> positions);

// Good: Virtual interface enforces constness; backend isolates C API cast
class SkPngEncoderBase {
    virtual bool onEncodeRow(SkSpan<const uint8_t> row) = 0;
};
```
