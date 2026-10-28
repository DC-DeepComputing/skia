---
name: skia/prefer-explicit-type-over-auto
description: When declaring variables for scalars, loop indices, range-for elements, container sizes, or non-obvious API returns, specify explicit concrete types instead of auto
rubric: readability
severity: should_fix
---

**Rule.** Specify variable types explicitly rather than using 'auto' when
declaring primitive counts, loop indices, range-for elements, container sizes,
or scalars (such as 'int', 'size_t', 'float'), avoiding 'auto' with literal
suffixes like '0ul'. Explicitly declare types when accessing member fields or
pointers, and when initializing from non-obvious returns like
'std::optional<T>', structs, fixed-size arrays, or spans. In documentation and
Fiddle examples, always spell out API return types explicitly. Reserve 'auto'
for iterators or where the type is already explicit in the initialization.

**Why.** Using 'auto' for primitive integers, loop variables, scalar constants,
container sizes, or non-obvious return types obscures bitwidth, signedness,
value semantics, and constness. Forcing unsigned loop indices via 'auto' with
suffixes like '0ul' deduces 'unsigned long' rather than canonical 'size_t',
risking cross-platform discrepancies. In documentation and tutorial examples,
'auto' obscures API return types for readers without IDE support. Explicit types
self-document contracts at a glance and prevent subtle comparison mismatches.

## Examples

Bad:

```cpp
// Obscures primitive scalar constant or integer type
auto tolerance = GrPathUtils::kDefaultTolerance;
auto numPlots = this->plotsPerPage();

// Obscures size_t signedness and bitwidth (or deduces non-portable unsigned long)
const auto count = pts.size();
for (auto s = 0ul, h = 0ul; s < softBreaksNum || h < hardBreaksNum; ) {}

// Obscures element type in range-based for loops
for (auto& line : commands) {
    if (line.find("<_skvm_jit>") >= 0) break;
}

// Obscures array type, dimensions, and copy semantics
auto pts = r.toQuad();

// Obscures third-party pointer vs native SkImage
auto image = fAvifDecoder->image;

// Obscures optionality and result struct fields
if (auto addRectResult = plot->addRect(size, image)) {
    doSomething(addRectResult->fID);
}

// Obscures API return type in documentation examples
auto minorAxis = matrix.mapVector({0, circleRadius});
```

Good:

```cpp
// Pass constant directly or spell out primitive scalar type
int vertexCount = GrAATriangulator::PathToAATriangles(
    path, GrPathUtils::kDefaultTolerance, clipBounds, &alloc);
int numPlots = this->plotsPerPage();

// Explicit size_t clarifies integer signedness, bitwidth, and portability
const size_t count = pts.size();
for (size_t s = 0, h = 0; s < softBreaksNum || h < hardBreaksNum; ) {}

// Explicit element type and const reference clarifies type and immutability
for (const SkString& line : commands) {
    if (line.find("<_skvm_jit>") >= 0) break;
}

// Explicit std::array documents element type and fixed length
std::array<SkPoint, 4> pts = r.toQuad();

// Explicit pointer clarifies third-party struct type
crabbyavif::avifImage* image = fAvifDecoder->image;

// Explicit type documents std::optional and struct type
if (std::optional<Plot::AddResult> addRectResult = plot->addRect(size, image)) {
    doSomething(addRectResult->fID);
}

// Explicit type makes API return obvious to doc readers
SkVector minorAxis = matrix.mapVector({0, circleRadius});
```
