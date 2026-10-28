---
name: skia/name-scalar-identifiers-with-singular-nouns
description: When naming scalar, count, or dimensional variables, use singular nouns for scalars and Count suffixes (e.g., strikeCount), and plural nouns for composite extents
rubric: readability
severity: nit
---

**Rule.** When declaring or refactoring parameters, variables, struct fields, or
constants of scalar or count type, avoid bare plural nouns and plural noun
adjuncts: use singular nouns for single items (e.g., ssboIndex, point, radius)
and explicit count forms (such as kNum<Items> or singular <item>Count, e.g.,
strikeCount, typefaceCount) rather than plural forms (e.g., strikesCount).
Conversely, when declaring variables holding 2D or multi-component dimensions
(such as SkSize, SkISize, SkVector, or LayerSpace<SkISize> representing
horizontal and vertical radius extents), use plural nouns (such as 'radii')
instead of singular nouns ('radius').

**Why.** Mismatched grammatical number on identifiers misleads readers and
callers about underlying data dimensions. Using plural nouns for scalars or
Count suffixes (e.g., strikesCount) wrongly implies a collection or violates
naming conventions, while naming 2D composite extents with singular 'radius'
conceals that the value contains distinct horizontal and vertical components.

## Examples

Bad:

```cpp
// Plural identifier for a scalar uint32_t or attribute descriptor
void writeVertices(DrawWriter*, const DrawParams&, uint32_t ssboIndices) const override;
constexpr Attribute kAttributes[] = {
    {"ssboIndices", VertexAttribType::kUInt, SkSLType::kUInt},
};

// Bare plural noun or plural noun adjunct for count variables
const int kSamples = 100;
const int strikesCount = buffer.readInt();

// Singular identifier for 2D composite radius dimensions
skif::LayerSpace<SkISize> radius =
        LayerSpace<SkSize>({3.f*sigma.width(), 3.f*sigma.height()}).ceil();
maxOutput.outset(radius);
```

Good:

```cpp
// Singular identifier matching scalar uint32_t type and attribute layout
void writeVertices(DrawWriter*, const DrawParams&, uint32_t ssboIndex) const override;
constexpr Attribute kAttributes[] = {
    {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
};

// Explicit kNum prefix or singular noun prefix before Count
const int kNumSamples = 100;
const int strikeCount = buffer.readInt();

// Plural identifier matching 2D composite radius dimensions
skif::LayerSpace<SkISize> radii =
        LayerSpace<SkSize>({3.f*sigma.width(), 3.f*sigma.height()}).ceil();
maxOutput.outset(radii);
```
