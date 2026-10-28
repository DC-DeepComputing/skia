---
name: skia/accurately-document-sentinel-returns
description: When documenting early returns or fallback branches returning default-constructed objects, describe their sentinel or placeholder semantics rather than claiming nothing is created
rubric: readability
severity: nit
---

**Rule.** When documenting early returns, fallback paths, or failure branches
that return default-constructed sentinel or placeholder objects, describe the
semantics of the returned object (e.g., that it is unattached, detached, or a
no-op instance) rather than stating that nothing is created or returned. Ensure
comments accurately reflect that a usable default instance is returned to
callers.

**Why.** Stating that a function 'does not create' or 'does not return' an
object when it actually returns a default-constructed sentinel or placeholder
contradicts the code and misleads readers. Accurately documenting that an
unattached or no-op instance is returned explains why callers can safely hold,
query, or pass the object without null checks or conditional guards.

## Examples

Bad:

```cpp
SkPDFStructTree::Mark SkPDFDocument::createMarkForElemId(int elemId, SkPoint p) {
    // If the mark isn't on a page (like when emitting a Type3 glyph) don't create a mark.
    if (!this->hasCurrentPage()) {
        return SkPDFStructTree::Mark();
    }
    return fStructTree.createMarkForElemId(elemId, SkToUInt(this->currentPageIndex()), p);
}
```

Good:

```cpp
SkPDFStructTree::Mark SkPDFDocument::createMarkForElemId(int elemId, SkPoint p) {
    // If the mark isn't on a page (like when emitting a Type3 glyph)
    // return a temporary mark not attached to the page or a structure element.
    if (!this->hasCurrentPage()) {
        return SkPDFStructTree::Mark();
    }
    return fStructTree.createMarkForElemId(elemId, SkToUInt(this->currentPageIndex()), p);
}
```
