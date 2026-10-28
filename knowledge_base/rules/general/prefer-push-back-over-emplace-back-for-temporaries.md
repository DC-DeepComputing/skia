---
name: skia/prefer-push-back-over-emplace-back-for-temporaries
description: When appending already-constructed temporary objects to containers, use push_back instead of emplace_back, reserving emplace_back for forwarding constructor arguments in place
rubric: readability
severity: nit
---

**Rule.** When appending an element to a container (such as std::vector or
skia_private::TArray), use push_back rather than emplace_back if the argument is
already an explicit temporary object (e.g., T{...} or Type(...)) or the result
of a factory call. Reserve emplace_back strictly for in-place construction where
constructor arguments are forwarded directly, and maintain consistent insertion
methods across related branches.

**Why.** emplace_back provides no performance advantage over push_back when
passed an already-materialized temporary object, as it must still move- or
copy-construct the element into container storage. Calling emplace_back with
explicit temporaries obscures intent, can inadvertently bypass explicit
constructor guards, and introduces arbitrary stylistic inconsistency when
adjacent code uses push_back.

## Examples

Bad:

```cpp
// Explicit temporary passed to emplace_back
eventOrdering.emplace_back(SetupTuple{s.upper().y, kBegin, -s.upper().x, s});

// Inconsistent insertion method across sibling branches
if (isHorizontal) {
    events.push_back(Event{point.x, kHorizontal});
} else {
    events.emplace_back(Event{point.y, kVertical});
}
```

Good:

```cpp
// Use push_back for explicit temporary objects
eventOrdering.push_back(SetupTuple{s.upper().y, kBegin, -s.upper().x, s});

// Consistent insertion across sibling branches
if (isHorizontal) {
    events.push_back(Event{point.x, kHorizontal});
} else {
    events.push_back(Event{point.y, kVertical});
}

// Reserve emplace_back for forwarding constructor arguments in place
events.emplace_back(point.y, kVertical);
```
