---
name: skia/name-container-elements-descriptively
description: When declaring local variables, boolean flags, collection aliases, or indices, use descriptive names rather than cryptic abbreviations, reserving single letters for loop counters
rubric: readability
severity: should_fix
---

**Rule.** When declaring local variables, loop variables, boolean flags,
collection aliases, or array and lookup table indices, use descriptive names
(such as 'shader', 'line', 'tile', 'column', 'colIndex', 'isVertical',
'windingBytes', or 'fillBytes') representing the entity, condition, indexed
dimension, or data representation rather than cryptic abbreviations or
single-letter identifiers (such as 's', 'l', 't', 'u', 'isV', 'wB', or 'fB'),
even if underlying helper functions or enums use abbreviated names (such as
'SkPixelGeometryIsV'). Reserve single-letter variable names strictly for
standard numeric loop induction counters (such as 'i', 'j', 'k') or spatial
coordinates (such as 'x', 'y', 'z').

**Why.** Cryptic abbreviations and single-letter variable names obscure an
object's type, role, condition, data representation, and semantic intent at call
sites—especially when using 'auto', evaluating predicates, or indexing into data
structures—forcing readers to search back to declarations. When indexing
multi-dimensional lookup tables or arrays, single-letter math symbols like 'u'
obscure which dimension or coordinate axis is accessed. Descriptive names make
element access, conditions, and data flow self-documenting.

## Examples

Bad:

```cpp
// Bad: Single-letter local variable used across multiple statements
auto s = SkImageShader::MakeForPaint(
        std::move(img), paint, SkTileMode::kClamp, SkTileMode::kClamp, sampling, &matrix);
if (!s) {
    return;
}
paintWithShader.setShader(std::move(s));

// Bad: Single-letter collection alias
const auto& l = rawLines[i];

// Bad: Single-letter parametric coordinate 'u' used as an array/LUT index
int u = std::clamp(tFixed >> 16, 0, Strip::kLutMaskWidthExcl);
uint8_t maskVal = maskRowLut[u];

// Bad: Cryptic abbreviations for SIMD vector representations or booleans
auto wB = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vSubsampleWinding);
bool isV = SkPixelGeometryIsV(props.pixelGeometry());
```

Good:

```cpp
// Good: Descriptive local variable name clarifies role across statements
auto shader = SkImageShader::MakeForPaint(
        std::move(img), paint, SkTileMode::kClamp, SkTileMode::kClamp, sampling, &matrix);
if (!shader) {
    return;
}
paintWithShader.setShader(std::move(shader));

// Good: Descriptive element alias
const auto& line = rawLines[i];

// Good: Descriptive dimension name for table index
int column = std::clamp(tFixed >> 16, 0, Strip::kLutMaskWidthExcl);
uint8_t maskVal = maskRowLut[column];

// Good: Descriptive variable names clearly convey data representation and condition
auto windingBytes = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vSubsampleWinding);
bool isVertical = SkPixelGeometryIsV(props.pixelGeometry());
```
