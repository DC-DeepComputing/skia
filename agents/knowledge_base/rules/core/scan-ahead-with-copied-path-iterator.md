---
name: skia/core/scan-ahead-with-copied-path-iterator
description: When path traversal, stroking, or rasterization requires contour-level knowledge like closure, scan ahead with a copied lightweight SkPathIter instead of buffering segments
rubric: design
severity: should_fix
---

**Rule.** When path iteration, stroking, or hairline rasterization requires
contour-level knowledge (such as determining whether a contour terminates with
kClose or kMove to style caps and joins), do not buffer initial segments or
defer drawing across verbs. Instead, scan ahead to the end of the contour using
a value-copied lightweight path iterator (such as SkPathIter or SkPath::Iter) to
discover closure or tangent properties upfront without dynamic allocations.

**Why.** Contour closure dictates whether end caps or joins connect contour
endpoints. Determining closure lazily while iterating segments forces code to
buffer first segments, defer drawing, and maintain bug-prone state machines
across all verb types. Value-copying a lightweight iterator like SkPathIter
incurs no dynamic allocations and lets routines inspect upcoming verbs upfront,
greatly simplifying downstream segment rendering.

## Examples

Bad:

```cpp
// Buffering the first segment and deferring drawing until the contour ends
for (auto iter = raw.iter(); auto rec = iter.next();) {
    if (isFirstLine) {
        std::copy(pts, pts + 2, firstLine);
        firstLineVerbType = SkPathVerb::kLine;
        break;
    }
    // ...
}
if (isLastVerb && isVerbALine) {
    draw_first_line();
}
```

Good:

```cpp
// Scanning ahead with a copied iterator to detect contour closure upfront
static inline bool is_next_contour_closed(SkPathIter scanner) {
    auto rec = scanner.next();
    if (rec->fVerb == SkPathVerb::kClose) return true;
    if (rec->fVerb == SkPathVerb::kMove) return false;

    while (scanner.peekNextVerb().has_value()) {
        SkPathVerb next_verb = scanner.peekNextVerb().value();
        if (next_verb == SkPathVerb::kMove) return false;
        if (next_verb == SkPathVerb::kClose) return true;
        scanner.next();
    }
    return false;
}
```
