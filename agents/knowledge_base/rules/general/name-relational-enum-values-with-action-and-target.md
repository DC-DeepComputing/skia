---
name: skia/name-relational-enum-values-with-action-and-target
description: When defining relational or geometric interaction enum values, specify both action and target entity, and name complex overlap explicitly (e.g. kComplexInteraction) rather than kIntersect
rubric: readability
severity: should_fix
---

**Rule.** When naming enum values that represent how one entity affects or
interacts with another, specify both the action verb and the affected target
entity (e.g., 'kClipsOutDraw' or 'kReplacesDraw' instead of bare verbs or
truncated phrases like 'kClipOut' or 'kClipOnly'). Additionally, for complex or
non-trivial geometric overlaps where elements partially interact without one
completely covering, clipping, or replacing the other, use explicit outcome
names (such as 'kComplexInteraction' or 'kComplexIntersection') rather than
generic operation names like 'kIntersect' that conflict with clip operations and
obscure that complex handling is required.

**Why.** Bare verb phrases and truncated nouns leave it ambiguous whether the
caller, argument, or target entity is the one acting or being acted upon.
Furthermore, generic names like 'kIntersect' are confusing in geometry and clip
simplification algorithms because clipping is already an intersection operation;
explicit names communicate the causal direction and make it clear when an
interaction requires complex or unresolved handling.

## Examples

Bad:

```cpp
// Ambiguous causal direction and generic operation name for complex overlap:
enum class DrawInfluence {
    kClipOut,    // Ambiguous whether draw clips element or vice versa
    kClipOnly,   // Unclear who is affected
    kNone,
    kIntersect,  // Generic name obscures that complex handling is needed
};
```

Good:

```cpp
// Explicit action and target entity, with explicit complex interaction outcome:
enum class DrawInfluence {
    kClipsOutDraw,        // The element causes the draw shape to be entirely clipped out
    kReplacesDraw,        // The element is fully covered, so the draw's shape can be ignored
    kNone,
    kComplexInteraction,  // The element and draw partially overlap and require detailed handling
};
```
