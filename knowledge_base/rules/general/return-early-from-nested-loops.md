---
name: skia/return-early-from-nested-loops
description: When an inner condition terminates a multi-level loop traversal or search, return directly (returning boolean literals if applicable) rather than maintaining intermediate boolean flags
rubric: readability
severity: should_fix
---

**Rule.** When an inner loop condition terminates an entire multi-level
traversal or search and no post-loop cleanup is needed, return directly from the
inner loop. Do not declare and maintain an intermediate boolean flag or
accumulator variable (such as found, done, hasPendingDraws, or result) solely to
break loops, control loop continuation (e.g. && !result), or assign within
return statements (return result = true;). In search or predicate functions,
return explicit boolean literals (return true; / return false;) directly.

**Why.** Returning directly eliminates unnecessary local state, simplifies
nested control flow, and avoids redundant loop condition checks, break
statements, or awkward assignments within return expressions when the search
outcome is already determined.

## Examples

Bad:

```cpp
// Breaking nested loops via flag:
for (int i = numPages - 1; i >= 0; --i) {
    bool hasPendingDraws = false;
    while (Plot* plot = plotIter.get()) {
        if (plot->inUse()) { hasPendingDraws = true; break; }
        plotIter.next();
    }
    if (hasPendingDraws) { break; }
    deactivatePage(i);
}

// Tracking redundant accumulator in search loops:
bool containsColor(SkTextBlob* blob) {
    bool result = false;
    while (!iter.done() && !result) {
        for (const SkGlyph* glyph : glyphs) {
            if (glyph->isColor()) { return result = true; }
        }
        iter.next();
    }
    return result;
}
```

Good:

```cpp
// Returning directly from nested loops:
for (int i = numPages - 1; i >= 0; --i) {
    while (Plot* plot = plotIter.get()) {
        if (plot->inUse()) { return; }
        plotIter.next();
    }
    deactivatePage(i);
}

// Returning boolean literals directly:
bool containsColor(SkTextBlob* blob) {
    while (!iter.done()) {
        for (const SkGlyph* glyph : glyphs) {
            if (glyph->isColor()) { return true; }
        }
        iter.next();
    }
    return false;
}
```
