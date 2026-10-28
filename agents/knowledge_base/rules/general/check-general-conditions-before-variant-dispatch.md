---
name: skia/check-general-conditions-before-variant-dispatch
description: When dispatching across enum variants or subtypes, evaluate universal edge cases or degenerate conditions upfront rather than coupling them to a specific variant branch
rubric: design
severity: should_fix
---

**Rule.** When processing polymorphic objects, AST nodes, or enum variants, do
not restrict an edge-case or degenerate condition check to a single variant
branch if that condition conceptually applies across multiple or all variants.
Evaluate universal conditions upfront before variant-specific branching or
dispatch, or handle them uniformly in a common fallback path.

**Why.** Narrowing a universal edge case to one specific variant creates
unnecessary coupling, duplicates degenerate handling across branches, misses
other variants exhibiting the same property, and can trigger downstream
invariant assertion failures. Checking common invariants upfront simplifies
control flow and ensures consistent behavior.

## Examples

Bad:

```cpp
if (SkPathVerb::kConic == verb) {
    ...
} else if (SkPathVerb::kCubic == verb && prevPt == pts[nextPt]) {
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
// Check universal degenerate condition upfront
if (prevPt == pts[nextPt]) {
    return false;
}
if (SkPathVerb::kConic == verb) {
    ...
} else if (!check_edge_against_rect(prevPt, pts[nextPt], rect, direction)) {
    return false;
}
```
