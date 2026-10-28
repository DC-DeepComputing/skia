---
name: skia/avoid-sktoint-to-silence-size-narrowing
description: When computing sizes or comparing size_t indices against int container sizes, avoid narrowing casts like SkToInt; use size_t or widen with SkToSizeT
rubric: readability
severity: should_fix
---

**Rule.** Do not add explicit narrowing casts such as `SkToInt`, `SkToU16`, or
`SkTo<...>` merely to silence compiler or IDE conversion warnings when computing
counts, sizes, pointer differences, or container offsets; update the function
signature or variable type to `size_t`, `ptrdiff_t`, or the logical
index/position type instead. Similarly, when comparing `size_t` indices or range
boundaries against container sizes returning `int` (such as `SkTArray::size()`),
widen the container size using `SkToSizeT(container.size())` rather than
narrowing `size_t` values with `SkToInt()`.

**Why.** Narrowing `size_t` values or pointer differences to smaller types like
signed `int` or `uint16_t` masks underlying type mismatches, risks silent
truncation on large inputs, and triggers debug assertion failures in `SkToInt`
if values exceed `INT_MAX`. Widening non-negative `int` container sizes to
`size_t` via `SkToSizeT` safely keeps comparisons at the natural width of the
unsigned index without risking index truncation.

## Examples

Bad:

```cpp
// Bad: Adding SkToInt merely suppresses narrowing warnings on an int-returning API
int SkFontPriv::CountTextElements(const void* text, size_t byteLength, SkTextEncoding encoding) {
    switch (encoding) {
        case SkTextEncoding::kUTF32:
            return SkToInt(byteLength >> 2);
        case SkTextEncoding::kGlyphID:
            return SkToInt(byteLength >> 1);
    }
}

// Bad: Narrowing pointer difference to uint16_t truncates on buffers > 64KB
uint16_t nextPos16 = SkToU16(start16 - utf16);

// Bad: Narrowing size_t range endpoints with SkToInt to compare with int container size
SkASSERT(SkToInt(blockRange.start) < fTextStyles.size() &&
         SkToInt(blockRange.end) <= fTextStyles.size());
```

Good:

```cpp
// Good: Update the API signature to return size_t directly
size_t SkFontPriv::CountTextElements(const void* text, size_t byteLength, SkTextEncoding encoding) {
    switch (encoding) {
        case SkTextEncoding::kUTF32:
            return byteLength >> 2;
        case SkTextEncoding::kGlyphID:
            return byteLength >> 1;
    }
}

// Good: Use the logical position/index type or ptrdiff_t
const auto nextPos16 = SkTo<SkBidiIterator::Position>(start16 - utf16);

// Good: Widen the non-negative int container size to size_t with SkToSizeT
SkASSERT(blockRange.start < SkToSizeT(fTextStyles.size()) &&
         blockRange.end <= SkToSizeT(fTextStyles.size()));
```
