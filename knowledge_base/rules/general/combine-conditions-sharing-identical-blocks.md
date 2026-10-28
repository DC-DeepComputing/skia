---
name: skia/combine-conditions-sharing-identical-blocks
description: When separate or sequential conditional statements execute identical execution blocks or fallback logic, combine their conditions using logical OR (||) into a single branch
rubric: design
severity: should_fix
---

**Rule.** When separate conditional statements (such as sequential if statements
or else-if ladders) execute identical processing or fallback blocks, combine
their conditions using logical OR (||) into a single compound condition rather
than duplicating the entire code block across branches.

**Why.** Duplicating code blocks across separate conditional checks increases
code churn, complicates maintenance, and risks divergence when updates are made
to only one copy. Additionally, in sequential if statements, keeping conditions
separate causes duplicate execution of potentially expensive operations when
both conditions evaluate to true.

## Examples

Bad:

```cpp
// Bad: Duplicating an identical processing block across separate sequential if checks
if (computeFromPath) {
    this->internalGetPath(glyph, alloc);
    if (const SkPath* path = glyph.path()) {
        GenerateMetricsFromPath(&glyph, *path, format, doVert);
    }
}
if (generateImageFromPath) {
    this->internalGetPath(glyph, alloc);
    if (const SkPath* path = glyph.path()) {
        GenerateMetricsFromPath(&glyph, *path, format, doVert);
    }
}
```

Good:

```cpp
// Good: Combine conditions with logical OR into a single branch
if (computeFromPath || generateImageFromPath) {
    this->internalGetPath(glyph, alloc);
    if (const SkPath* path = glyph.path()) {
        GenerateMetricsFromPath(&glyph, *path, format, doVert);
    }
}
```
