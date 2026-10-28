---
name: skia/prefer-designated-initializers-for-aggregates
description: When initializing aggregate structs, initialize members at declaration using C++20 designated initializers ({.field = value}) rather than separate field assignments or positional initialization
rubric: readability
severity: should_fix
---

**Rule.** When constructing or initializing aggregate structs, initialize
members directly at declaration using C++20 designated initializers (`{.field =
value}`). Do not default-construct an aggregate struct to assign its fields
across separate subsequent statements, and avoid fragile positional aggregate
initialization (`{a, b, ...}`) where members can silently misbind when fields
are reordered or defaulted. Ensure designators follow member declaration order
as required by C++20.

**Why.** Initializing aggregate structs directly at declaration eliminates
boilerplate lines of individual member assignments and prevents leaving
uninitialized or partially initialized state. Using C++20 designated
initializers makes field initialization self-documenting, prevents silent member
transposition bugs across identical types, and catches member renamings or
reorderings at compile time.

## Examples

Bad:

```cpp
// Bad: Default-constructing and assigning members individually
SkPDFTagNode::AnnotationInfo annotationInfo;
annotationInfo.fPageIndex = pageIndex;
annotationInfo.fAnnotationRef = annotationRef;

// Bad: Fragile positional aggregate initialization
DecodingDstInfo fullImageDecodingDst = {decodingState.fPreblendBuffer,
                                        this->getEncodedRowBytes(),
                                        this->getEncodedRowBytes(),
                                        encodedBytesPerPixel};
```

Good:

```cpp
// Good: Designated aggregate initialization at declaration
SkPDFTagNode::AnnotationInfo annotationInfo = {
    .fPageIndex = pageIndex,
    .fAnnotationRef = annotationRef,
};

DecodingDstInfo fullImageDecodingDst = {
    .fDst = decodingState.fPreblendBuffer,
    .fDstRowStride = this->getEncodedRowBytes(),
    .fDstRowSize = this->getEncodedRowBytes(),
    .fDstBytesPerPixel = encodedBytesPerPixel,
};
```
