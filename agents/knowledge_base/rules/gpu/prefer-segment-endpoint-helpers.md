---
name: skia/gpu/prefer-segment-endpoint-helpers
description: When defining or checking geometric segment and edge vertex endpoints, name matching predicates isEndpoint rather than contains, and avoid manual pairwise comparisons
rubric: readability
severity: should_fix
---

**Rule.** When declaring query methods on line segment or edge primitives that
test whether a point or vertex matches an endpoint, name the predicate
isEndpoint(point) (or hasEndpoint) rather than contains(point). At call sites
checking whether two segments or edges share endpoints, call
segment->isEndpoint(...) rather than expanding manual four-way pairwise vertex
equality comparisons.

**Why.** In geometric primitives, contains(point) conventionally denotes
containment testing whether a point lies along the segment or within the shape's
interior, whereas isEndpoint(point) clearly conveys exact vertex identity
matching. Using dedicated isEndpoint() helpers also eliminates verbose,
error-prone combinatorial equality checks (such as fTop == other.fTop || fBottom
== other.fBottom) across call sites.

## Examples

Bad:

```cpp
// Bad: naming endpoint check 'contains', and open-coding 4-way pairwise equality
bool contains(const SkPoint& point) const {
    return (point == fTop->fPoint) || (point == fBottom->fPoint);
}

if (fOriginalSegment && other.fOriginalSegment && (
        fOriginalSegment->fTop == other.fOriginalSegment->fTop ||
        fOriginalSegment->fBottom == other.fOriginalSegment->fBottom ||
        fOriginalSegment->fTop == other.fOriginalSegment->fBottom ||
        fOriginalSegment->fBottom == other.fOriginalSegment->fTop)) {
    return false;
}
```

Good:

```cpp
// Good: naming predicate 'isEndpoint' and using it at call sites
bool isEndpoint(const SkPoint& point) const {
    return (point == fTop->fPoint) || (point == fBottom->fPoint);
}

if (fOriginalSegment && other.fOriginalSegment && (
        fOriginalSegment->isEndpoint(other.fOriginalSegment->fTop) ||
        fOriginalSegment->isEndpoint(other.fOriginalSegment->fBottom))) {
    return false;
}
```
