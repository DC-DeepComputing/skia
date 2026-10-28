---
name: skia/gpu/avoid-redundant-parent-member-caches
description: When state is encapsulated in an embedded struct or referenced parent object, access properties directly rather than declaring duplicate member fields or redundant accessors
rubric: design
severity: should_fix
---

**Rule.** When an object embeds an aggregate parameter or component struct (such
as DrawParams) or an inner helper class references an enclosing parent object,
access state directly through that associated object. Do not declare duplicate
member variables, cached references, or redundant accessors in the outer class
or helper when the data is already stored in the embedded struct or parent
object.

**Why.** Duplicating member fields across enclosing classes, embedded structs,
or helper classes wastes memory, introduces redundant accessor boilerplate, and
creates synchronization hazards where the duplicate copies can drift out of
sync. Treating the encapsulated struct or parent object as the single source of
truth ensures consistency and simplifies object layout.

## Examples

Bad:

```cpp
// Bad: Helper class redundantly caches parent fields
class DrawWriter::Appender {
    Appender(DrawWriter& w)
            : fDrawer(w), fAppendVerts(w.fRenderState & kAppend), fTarget(w.fAppend) {}
    DrawWriter& fDrawer;
    BindBufferInfo& fTarget; // Redundant cached reference
};

// Bad: Enclosing class duplicates embedded parameter fields
struct Draw {
    Draw(..., BarrierType b) : fDrawParams(..., b), fBarrierBeforeDraws(b) {}
    const BarrierType& barrierBeforeDraws() const { return fBarrierBeforeDraws; }
    DrawParams fDrawParams;
    BarrierType fBarrierBeforeDraws; // Redundant duplicate field
};
```

Good:

```cpp
// Good: Query parent state directly via parent reference
class DrawWriter::Appender {
    Appender(DrawWriter& w) : fDrawer(w), fAppendVerts(w.fRenderState & kAppend) {}
    DrawWriter& fDrawer; // Access fDrawer.fAppend directly
};

// Good: Enclosing class exposes and queries state through embedded struct
struct Draw {
    Draw(..., BarrierType b) : fDrawParams(..., b) {}
    const DrawParams& drawParams() const { return fDrawParams; }
    DrawParams fDrawParams;
};
// Callers query draw.drawParams().barrierBeforeDraws()
```
