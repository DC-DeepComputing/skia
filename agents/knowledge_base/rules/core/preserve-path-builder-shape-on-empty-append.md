---
name: skia/core/preserve-path-builder-shape-on-empty-append
description: When appending geometry into path builders or accumulators, preserve tracked shape classification and convexity metadata when the source path contains no verbs
rubric: correctness
severity: should_fix
---

**Rule.** When appending geometry from an incoming path or buffer into a path
builder or accumulator, do not reset or degrade tracked shape classification
metadata (such as fType or fIsA) when the incoming source contains no verbs.
Guard shape degradation so that appending an empty path remains a no-op that
preserves existing primitive shape recognition.

**Why.** Appending a path with no verbs adds no geometric contours or segments.
Unconditionally resetting shape classification on empty appends discards
existing shape optimization metadata (such as convexity and primitive
classification) and degrades the builder to a general unclassified path type.

## Examples

Bad:

```cpp
std::tuple<SkPoint*, SkScalar*> SkPathBuilder::growForVerbsInPath(const SkPathRef& path) {
    fSegmentMask |= path.fSegmentMask;
    fIsA = SkPathBuilder::IsA::kIsA_MoreThanMoves;

    if (int numVerbs = path.countVerbs()) {
        memcpy(fVerbs.push_back_n(numVerbs), path.fVerbs.begin(), numVerbs * sizeof(fVerbs[0]));
    }
    ...
}
```

Good:

```cpp
std::tuple<SkPoint*, SkScalar*> SkPathBuilder::growForVerbsInPath(const SkPathRef& path) {
    fSegmentMask |= path.fSegmentMask;

    if (int numVerbs = path.countVerbs()) {
        fIsA = SkPathBuilder::IsA::kIsA_MoreThanMoves;
        memcpy(fVerbs.push_back_n(numVerbs), path.fVerbs.begin(), numVerbs * sizeof(fVerbs[0]));
    }
    ...
}
```
