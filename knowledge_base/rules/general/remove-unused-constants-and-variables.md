---
name: skia/remove-unused-constants-and-variables
description: When refactoring or authoring code and tests, delete unreferenced constants, unused variables, dead member fields, redundant flags, and obsolete no-op query calls
rubric: readability
severity: should_fix
---

**Rule.** Do not leave unused or write-only local variables, unreferenced
constants, lookup tables, dead allocations masked by null checks, unreferenced
test boilerplate, or obsolete calculations and no-op query calls left passing
nullptr for eliminated output buffers in source or test files. In classes and
structs, remove unused member fields, unneeded constructor parameters, dead
method overrides, and unreferenced debug declarations (such as fields in
SkDEBUGCODE); if eliminating dead fields leaves a single primitive member,
simplify the struct to the primitive type or a type alias. When deleting an
interface or data structure, delete private fields, bookkeeping logic, and
obsolete calls that populated them, removing redundant or dead flags.

**Why.** Unused constants, write-only local variables, dead allocations,
copy-pasted test boilerplate, orphaned bookkeeping state, unreferenced
variables, dead or redundant flags, and unused debug declarations clutter code,
increase object memory footprint, and mislead readers into believing state or
loop progress is actively tracked. Similarly, retaining query calls with nullptr
out-parameters after deleting the buffers they populated leaves dead code that
wastes CPU cycles on unneeded calculations (such as strike resolution or metrics
lookups). In tests, leftover unreferenced objects confuse reviewers into
wondering if intended assertions or draw calls were omitted and can cause
resource leaks. In templated classes, loop bodies with write-only counters,
under debug macros like SkDEBUGCODE, or when variables are tested for null or
passed by address to out-parameter APIs (such as mapPoints), compiler
diagnostics do not always trigger, allowing dead code and resource allocations
to persist undetected. Simplifying single-member structs to primitive types or
type aliases eliminates unnecessary abstraction boilerplate.

## Examples

Bad:

```cpp
// Write-only local variable or abandoned loop counter
for (auto width : widths) {
    int i = 0;
    for (auto cap : caps) {
        drawPath(canvas, cap, width);
    }
    i++;
}

// Dead allocation masked by null check
auto surface = canvas->makeSurface(canvas->imageInfo().makeWH(80, 80));
if (!surface) {
    return DrawResult::kSkip;
}
drawScaledPath(canvas, path, paint);

// Dead calculation populated via out-parameter or dead call passing nullptr after buffer removal
SkPoint center;
m.mapPoints(&center, &localCenter, 1);
float radius = m.mapRadius(localOuterRadius);
fFont.getBounds(fGlyphs.data(), fGlyphs.size(), /*fBounds.data()*/nullptr, nullptr);

// Unused test fixture / dead setup boilerplate copied from another test
gm('paragraph_client_icu', (canvas) => {
    const paint = new CanvasKit.Paint(); // Never used or deleted
    paint.setColor(CanvasKit.RED);
    const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
});

// Unused debug-only member variable or redundant flag
class Tiler {
    SkTDArray<Tile> fTileBuf;
    SkDEBUGCODE(bool fSorted;) // Never referenced or asserted
    bool fUpdated; // Never updated to true, creating dead checks
};

// Unused struct field leaving single-member wrapper struct
struct ImmutableValue {
    Type::NumberKind fKind;  // Never inspected after analysis
    int32_t fBits;
};
bool pushPreexistingImmutableData(const TArray<ImmutableValue>& values);
```

Good:

```cpp
// Write-only counter removed
for (auto width : widths) {
    for (auto cap : caps) {
        drawPath(canvas, cap, width);
    }
}

// Dead allocation and unused null check removed
drawScaledPath(canvas, path, paint);

// Unused out-parameter calculation and dead query call removed entirely
float radius = localOuterRadius * transform.maxScaleFactor();

// Clean test setup retaining only actively used fixtures
gm('paragraph_client_icu', (canvas) => {
    const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
});

// Dead debug declarations and redundant flags removed
class Tiler {
    SkTDArray<Tile> fTileBuf;
    bool isDrawn() const { return !fTileBuf.empty(); }
};

// Dead field eliminated and single-member wrapper simplified to type alias
using ImmutableBits = int32_t;
bool pushPreexistingImmutableData(const TArray<ImmutableBits>& values);
```
