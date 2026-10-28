---
name: skia/modules/guard-boundary-element-access-with-conjunction
description: When querying properties of boundary elements in ranges or containers, guard the access with conjunction (!empty() && ...) rather than disjunction (empty() || ...)
rubric: correctness
severity: should_fix
---

**Rule.** When testing whether a range or container satisfies a property of its
boundary or trailing element (such as back() or end - 1), guard the element
lookup using a non-empty conjunction (!empty() && ... or width() > 0 && ...). Do
not use disjunction (empty() || ...), which causes empty collections to evaluate
to true and falsely satisfy the predicate.

**Why.** An empty range or container does not possess the property of an absent
trailing element (such as ending in a newline or hard line break). Guarding with
disjunction causes the expression to short-circuit to true on empty inputs,
introducing logic errors where empty collections are treated as matching the
element predicate.

## Examples

Bad:

```cpp
// Bug: returns true when the range is empty
return fGhostClusterRange.end == fGhostClusterRange.start ||
       fOwner->cluster(fGhostClusterRange.end - 1).isHardBreak();
```

Good:

```cpp
// Evaluates to false when the range is empty
return (fGhostClusterRange.width() > 0 &&
        fOwner->cluster(fGhostClusterRange.end - 1).isHardBreak());
```
