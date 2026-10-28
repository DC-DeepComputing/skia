---
name: skia/iterate-directly-over-path-iter-points-span
description: When querying path verb point counts or inspecting path iterators, use SkPathPriv::PtsInIter/PtsInVerb or range-for over rec->fPoints instead of hand-rolled switches or tables
rubric: readability
severity: should_fix
---

**Rule.** When querying the number of points for a path verb (SkPathVerb or
SkPath::Verb), use existing SkPathPriv helpers (such as SkPathPriv::PtsInIter or
SkPathPriv::PtsInVerb) instead of writing custom switch functions or local
lookup tables. When inspecting or processing points from modern
SkPath::Iter::next() or SkPath::RangeIter, iterate directly over rec->fPoints
using a range-based for loop rather than indexing through an auxiliary
verb-to-point-count table or manual index loop.

**Why.** Skia provides centralized lookup tables in SkPathPriv (PtsInIter for
iteration points including starter points, and PtsInVerb for raw verb points);
hand-rolling local switches or lookup arrays duplicates boilerplate and adds
maintenance overhead. Additionally, modern path iterators return sized spans
(rec->fPoints) that already encapsulate point counts, making auxiliary count
lookup tables and manual index tracking obsolete.

## Examples

Bad:

```cpp
// Bad: hand-rolling a verb-to-points switch
static int verb_to_points(SkPath::Verb verb) {
    switch (verb) {
        case SkPath::kMove_Verb:  return 1;
        case SkPath::kLine_Verb:  return 2;
        case SkPath::kQuad_Verb:  return 3;
        case SkPath::kConic_Verb: return 3;
        case SkPath::kCubic_Verb: return 4;
        case SkPath::kClose_Verb: return 0;
        case SkPath::kDone_Verb:  return 0;
    }
    SkUNREACHABLE;
}

// Bad: using an auxiliary pointCount array to iterate modern iter points
const int pointCount[] = { 1, 2, 3, 3, 4, 1, 0 };
while (auto rec = iter.next()) {
    for (int i = 0; i < pointCount[(int)rec->fVerb]; ++i) {
        process(rec->fPoints[i]);
    }
}
```

Good:

```cpp
// Good: use centralized SkPathPriv helpers for verb point counts
const int numPts = SkPathPriv::PtsInIter(verbA);

// Good: iterate directly over rec->fPoints using a range-for loop
while (auto rec = iter.next()) {
    for (const SkPoint& p : rec->fPoints) {
        process(p);
    }
}
```
