---
name: skia/prefer-skspan-subspan-over-pointer-arithmetic
description: When slicing contiguous buffers or extracting sub-spans, use SkSpan slicing methods (first, last, subspan) instead of raw pointer arithmetic, SkTAddOffset, or ad-hoc helpers
rubric: safety
severity: should_fix
---

**Rule.** When slicing contiguous buffers, extracting sub-ranges, or indexing
row strides, use SkSpan and built-in slicing methods such as first(), last(), or
subspan() rather than raw pointer arithmetic (e.g. ptr + offset, span.data() +
offset, or SkTAddOffset), tracked index offsets, ad-hoc helper templates (such
as pin_size), or manual {span.data(), count} reconstructions. Ensure buffer
slicing relies on built-in span bounds validation to prevent out-of-bounds reads
and writes.

**Why.** Raw pointer arithmetic (ptr + offset, span.data() + offset,
SkTAddOffset) and custom span-slicing helpers bypass SkSpan's built-in bounds
assertions (via sk_collection_check_length) or add redundant boilerplate. Using
standard slicing methods like first(), last(), and subspan() clearly conveys
slicing intent, avoids code duplication, and enforces bounds checks against
buffer overruns.

## Examples

Bad:

```cpp
// Bad: Manual pointer arithmetic and SkTAddOffset for row slicing
const uint8_t* srcRow = srcBase + (y * srcRowBytes);
void* dstRow = SkTAddOffset<void>(dst, dstY * dstRowStride);
this->swizzleRow(srcRow, dstRow);

// Bad: Ad-hoc helper reconstructing a span from data() and size
template <typename T> SkSpan<T> pin_size(SkSpan<T> span, size_t limit) {
    return { span.data(), std::min(span.size(), limit) };
}
for (auto glyph : pin_size(glyphs, bounds.size())) { ... }
```

Good:

```cpp
// Good: Slice memory buffers using subspan() with validated bounds
SkSpan<const uint8_t> srcRow = srcImage.subspan(y * srcRowBytes, srcRowBytes);
SkSpan<uint8_t> dstRow = dstImage.subspan(dstY * dstRowStride, dstRowStride);
this->swizzleRow(srcRow.data(), dstRow.data());

// Good: Use built-in first() directly
for (auto glyph : glyphs.first(std::min(glyphs.size(), bounds.size()))) { ... }
```
