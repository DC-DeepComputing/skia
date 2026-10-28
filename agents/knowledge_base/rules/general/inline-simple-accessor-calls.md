---
name: skia/inline-simple-accessor-calls
description: When using concise accessors, lookups, casts, wrapper arguments, or direct return expressions like make_unique, inline them directly, but hoist complex math or loop bounds
rubric: readability
severity: should_fix
---

**Rule.** Do not declare redundant single-use intermediate local variables or
reference aliases to hold trivial member accesses, container sizes, array
lookups, accessors, casts, wrapper objects, or return expressions when they can
be cleanly evaluated inline in function calls, assertions, return statements, or
loop conditions. In particular, return expressions directly (such as boolean
checks or std::make_unique<T>(...)) and inline conditions into early-return
checks instead of creating temporary variables. Conversely, extract non-trivial
loop start or boundary expressions involving math functions or multi-step
rounding casts (such as static_cast<int32_t>(std::ceil(...))) into a descriptive
named local variable.

**Why.** Inlining simple getters, container sizes, array lookups,
straightforward casts, wrapper arguments, return expressions (like
std::make_unique), and boolean expressions directly into calls, assertions,
returns, and conditions eliminates visual noise and unnecessary variable scope.
Conversely, embedding complex math computations or multi-step casts inside a
loop header clutters the loop statement; defining a descriptive local variable
upfront clarifies intent.

## Examples

Bad:

```cpp
// Redundant temporary boolean variables for if check and return
bool isAvif = avifPeekCompatibleFileType(&avifData) == AVIF_TRUE;
if (isAvif) return true;
...
return isAvif;

// Redundant intermediate variable solely to return on the next line
auto sampler = std::make_unique<DrawDownSampler>();
return sampler;

// Redundant intermediate variable for dictionary or subscript lookup
const skjson::ObjectValue* jcolor = jpaint["c"];
abuilder->dispatchColorProperty(color_node, jcolor);

// Redundant intermediate variable for trivial loop bound
size_t childCount = node->fChildCount;
for (size_t i = 0; i < childCount; ++i) {
    process(node->fChildren[i]);
}

// Redundant temporary aliases for cheap getter and argument casts
const Rect& drawBounds = clip.drawBounds();
SkASSERT(!drawBounds.isEmptyNegativeOrNaN());
SkColor actual = pixmap.getColor((int)pt.x(), (int)pt.y());

// Cluttered loop header with embedded math and rounding cast
for (int32_t row = static_cast<int32_t>(std::ceil(yEdge)); row < kTileHeight; ++row) {
    processRow(row);
}
```

Good:

```cpp
// Inlined directly into early exit and return statement
if (crabbyavif::avifPeekCompatibleFileType(&avifData) == crabbyavif::CRABBY_AVIF_TRUE) {
    return true;
}
...
return bytesRead >= 12 && !memcmp(&bytes[4], "ftyp", 4);

// Return expression directly without intermediate local variable
return std::make_unique<DrawDownSampler>();

// Inlined dictionary lookup directly into call argument
abuilder->dispatchColorProperty(color_node, jpaint["c"]);

// Inlined directly into loop condition
for (size_t i = 0; i < node->fChildCount; ++i) {
    process(node->fChildren[i]);
}

// Inlined directly into assertion, call site, and wrapper argument
SkASSERT(!clip.drawBounds().isEmptyNegativeOrNaN());
SkColor actual = pixmap.getColor(pt.x(), pt.y());

// Complex loop bound hoisted to descriptive named local variable
int32_t startY = static_cast<int32_t>(std::ceil(yEdge));
for (int32_t row = startY; row < kTileHeight; ++row) {
    processRow(row);
}
```
