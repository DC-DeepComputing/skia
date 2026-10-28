---
name: skia/modules/consistent-sweep-line-endpoint-coordinates
description: When constructing event tuples or sort keys for segment endpoints in sweep-line algorithms, use coordinates matching the endpoint being processed rather than mixing opposite endpoints
rubric: readability
severity: should_fix
---

**Rule.** When constructing event tuples, sort keys, or ordering records for
segment endpoints in sweep-line algorithms, consistently use the coordinates of
the endpoint being processed (e.g., lower().x for an end event at lower().y). Do
not mix coordinates from opposite endpoints (such as pairing lower().y with
upper().x) for tie-breaking.

**Why.** Mixing coordinates from opposite endpoints creates the appearance of a
copy-paste bug and confuses reviewers, even when any arbitrary total ordering is
mathematically sufficient for tie-breaking. Consistently using the coordinates
corresponding to the active endpoint keeps the event representation intuitive
and self-consistent.

## Examples

Bad:

```cpp
eventOrdering.emplace_back(s, s.lower().y, kEnd, -s.upper().x);
```

Good:

```cpp
eventOrdering.emplace_back(s, s.lower().y, kEnd, -s.lower().x);
```
