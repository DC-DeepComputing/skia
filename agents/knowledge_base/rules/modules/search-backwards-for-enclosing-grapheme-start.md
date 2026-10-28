---
name: skia/modules/search-backwards-for-enclosing-grapheme-start
description: When determining the grapheme range enclosing a cluster or coordinate in SkParagraph, find the start boundary with findPreviousGraphemeBoundary rather than findNextGraphemeBoundary
rubric: correctness
severity: should_fix
---

**Rule.** When determining the grapheme range enclosing a text cluster or
coordinate in SkParagraph, resolve the start boundary with a backward search
using findPreviousGraphemeBoundary() and the end boundary with a forward search
using findNextGraphemeBoundary(). Do not use a forward boundary search for both
endpoints.

**Why.** To determine the bounding range enclosing an index, the start boundary
must be located by searching backwards to preceding boundary markers. Using a
forward search for the start boundary skips preceding boundaries, failing to
enclose the index and producing invalid grapheme widths.

## Examples

Bad:

```cpp
TextIndex graphemeUtf8Start =
    fOwner->findNextGraphemeBoundary(clusterIndex8);
TextIndex graphemeUtf8Width =
    fOwner->findNextGraphemeBoundary(clusterEnd8) - graphemeUtf8Start;
```

Good:

```cpp
TextIndex graphemeUtf8Start =
    fOwner->findPreviousGraphemeBoundary(clusterIndex8);
TextIndex graphemeUtf8Width =
    fOwner->findNextGraphemeBoundary(clusterEnd8) - graphemeUtf8Start;
```
