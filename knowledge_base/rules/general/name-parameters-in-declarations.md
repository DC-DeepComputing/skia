---
name: skia/name-parameters-in-declarations
description: When declaring and defining functions or callbacks, name parameters descriptively rather than using cryptic abbreviations, and keep names consistent across declarations and definitions
rubric: readability
severity: should_fix
---

**Rule.** Explicitly name all parameters in function declarations, definitions,
function pointer types, and callback signatures rather than leaving them
anonymous or using cryptic single-letter identifiers and abbreviations (such as
'b', 'bc', or loop induction names like 'i' or 'j'). Provide clear, descriptive
parameter names (such as 'bones', 'boneCount', 'quadIdx', 'offset', or 'size')
conveying each argument's semantic role, especially when adjacent parameters
share the same type or doc comments reference them by name (|src|, @param).
Ensure parameter names and their ordering match consistently between
declarations, definitions, doc comments, and forwarded helper methods when
representing the same concept to prevent inverted arguments and tracing
confusion.

**Why.** Anonymous parameters and cryptic single-letter names obscure argument
meaning, while omitting names referenced in doc comments breaks documentation
links. Mismatched parameter names or swapped ordering between declarations,
definitions, and forwarded helper methods—especially for adjacent parameters of
identical types (such as width/height or columns/rows)—obscure dataflow, confuse
callers, and trigger static analysis diagnostics (e.g., clang-tidy
readability-inconsistent-declaration-parameter-name).

## Examples

Bad:

```cpp
// Anonymous parameters in declarations obscure argument meaning and break doc comment references:
/** Encode the |src| pixels to the |dst| stream. */
SK_API bool Encode(SkWStream*, const SkPixmap&);

// Single-letter names or cryptic abbreviations obscure parameter roles:
void SkXPSDevice::drawVertices(const SkVertices* v, const SkMatrix* b, int bc,
                               SkBlendMode blendMode, const SkPaint& paint);

// Single-letter index parameters fail to convey what entity is indexed:
void outputLinesFromQuadSimd(uint32_t i, float x0, float dx, uint32_t numSegments,
                             uint32_t startIdx);

// Parameter name or ordering mismatch between declaration and definition:
// Declaration:
void assembleMatrixFromExpressions(const AnyConstructor& ctor, int rows, int columns);
// Definition:
void MetalCodeGenerator::assembleMatrixFromExpressions(const AnyConstructor& ctor,
                                                       int columns, int rows) { ... }
```

Good:

```cpp
// Named parameters align with doc comments and clarify argument roles:
/** Encode the |src| pixels to the |dst| stream. */
SK_API bool Encode(SkWStream* dst, const SkPixmap& src);

// Descriptive parameter names clearly convey argument purpose and semantics:
void SkXPSDevice::drawVertices(const SkVertices* vertices, const SkMatrix* bones, int boneCount,
                               SkBlendMode blendMode, const SkPaint& paint);

// Descriptive names clarify the indexed entity:
void outputLinesFromQuadSimd(uint32_t quadIdx, float x0, float dx, uint32_t numSegments,
                             uint32_t startIdx);

// Parameter names and ordering match consistently between declaration and definition:
// Declaration:
void assembleMatrixFromExpressions(const AnyConstructor& ctor, int columns, int rows);
// Definition:
void MetalCodeGenerator::assembleMatrixFromExpressions(const AnyConstructor& ctor,
                                                       int columns, int rows) { ... }
```
