---
name: skia/sync-parameter-comments-with-signatures
description: When declaring, modifying, or porting function signatures, document parameter semantics and defaults, and adapt comments to reflect active types while removing stale comments
rubric: readability
severity: should_fix
---

**Rule.** When declaring, modifying, or porting function signatures (such as
introducing optional arguments, std::optional<T>, default values, or TypeScript
declarations), document parameter semantics, specify fallback behavior when
omitted, and document coupling conditions with configuration flags or enum
masks. Additionally, update or remove stale doc comments, eliminated @param
tags, obsolete wrapping descriptions, outdated @return descriptions, and native
C++ enum or type references to reflect active types, parameter names, constness,
mutability, and defaults.

**Why.** Stale comments, obsolete @param tags, blank parameter tags, or omitted
fallback descriptions leave callers guessing what values are expected, what
default behavior occurs when optional arguments are omitted, and how optional
arguments interact with accompanying flag masks. Keeping parameter comments,
defaults, flag requirements, and return descriptions accurate prevents caller
errors and ensures Doxygen and TypeScript declaration docs remain reliable.

## Examples

Bad:

```cpp
// Variant 1: Stale note on refactored parameter type/constness:
void generateVertexSkSL(const Caps*, const RenderStep*, const SharedGeneratorData&);

// Variant 2: Stale doc comment contradicting newly parameterized default argument:
/** Constructs an empty SkPath. FillType is set to kWinding. */
SkPath(SkPathFillType = SkPathFillType::kDefault);

// Variant 3: Empty @param tags omitting fallback behavior for optional parameters:
/** @param sampleCount \n @param stencil */
MakeOnScreenGLSurface(ctx: GrDirectContext, width: number, height: number,
                      colorSpace: ColorSpace, sampleCount?: number, stencil?: number): Surface | null;

// Variant 4: Stale C++ enum/type references in ported TypeScript doc comments:
/** Circle begins at (x + radius, y), continuing clockwise if dir is kCW_Direction, and counterclockwise if dir is kCCW_Direction. */
addCircle(x: number, y: number, r: number, isCCW?: boolean): Path;
```

Good:

```cpp
// Variant 1: Removed stale comment contradicting const reference:
void generateVertexSkSL(const Caps*, const RenderStep*, const SharedGeneratorData&);

// Variant 2: Doc comment updated to reflect default parameter:
/** Constructs an empty SkPath: no verbs, no points, no conic weights. */
explicit SkPath(SkPathFillType = SkPathFillType::kDefault);

// Variant 3: Documented parameter semantics and fallback behavior when omitted:
/**
 * @param sampleCount - Sample count from GL_SAMPLES; if omitted, looked up from canvas.
 * @param stencil - Stencil count from GL_STENCIL_BITS; if omitted, looked up from WebGL context.
 */
MakeOnScreenGLSurface(ctx: GrDirectContext, width: number, height: number,
                      colorSpace: ColorSpace, sampleCount?: number, stencil?: number): Surface | null;

// Variant 4: Adapted doc comment reflecting TypeScript parameter names and semantics:
/**
 * Adds circle centered at (x, y) of size radius to the path.
 * @param isCCW - if the path should be drawn counter-clockwise or not
 */
addCircle(x: number, y: number, r: number, isCCW?: boolean): PathBuilder;
```
