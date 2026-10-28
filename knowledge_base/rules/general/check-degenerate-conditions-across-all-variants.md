---
name: skia/check-degenerate-conditions-across-all-variants
description: When bailing out on degenerate conditions like coincident endpoints, check the condition universally across all variants rather than gating on a specific verb
rubric: correctness
severity: should_fix
---

**Rule.** When bailing out on degenerate or invalid geometric conditions (such
as coincident endpoints or zero-length chords), check the degenerate condition
directly across all variants rather than restricting the check to a specific
enum or curve type (e.g., cubic curves) where the failure was observed.
Structure the guard upfront before subtype dispatch instead of gating on one
variant and relying on assertions or unchecked fallthrough for remaining cases.

**Why.** Restricting degenerate condition checks to a specific subtype leaves
other curve types unprotected or dependent on fragile assertions, even though
the underlying failure mode (such as undefined tangent directions or zero-length
edges) applies universally. Testing the condition upfront simplifies control
flow and ensures robust handling across all geometry variants.

## Examples

Bad:

```cpp
} else if (SkPathVerb::kCubic == verb && prevPt == pts[nextPt]) {
    // Only handling zero-length chords for cubics
    return false;
} else {
    SkASSERT(prevPt != pts[nextPt]);
    if (!check_edge_against_rect(prevPt, pts[nextPt], rect, direction)) {
        return false;
    }
}
```

Good:

```cpp
if (prevPt == pts[nextPt]) {
    // Start and end points are identical; no valid non-zero length edge across all verbs.
    return false;
} else if (SkPathVerb::kConic == verb) {
    ...
} else {
    if (!check_edge_against_rect(prevPt, pts[nextPt], rect, direction)) {
        return false;
    }
}
```
