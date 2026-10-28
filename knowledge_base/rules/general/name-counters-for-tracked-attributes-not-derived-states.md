---
name: skia/name-counters-for-tracked-attributes-not-derived-states
description: When maintaining counters or flags tracking attached handlers or resource attributes, name them after the concrete attribute directly measured rather than derived dynamic states
rubric: readability
severity: should_fix
---

**Rule.** Name counters, metrics, and status flags after the concrete attribute,
pointer presence, or attached handler directly being tracked (such as
fPinnerCount) rather than a presumed downstream dynamic state (such as
fUnpinnedCount). Do not conflate the presence or absence of an attachment with
an operational state when attached objects dynamically evaluate or modify that
state at runtime.

**Why.** Naming a counter after a downstream dynamic state is inaccurate when
entries with attached handlers may still satisfy the condition at runtime (such
as via canDelete()). Tracking the concrete attribute or handler presence
directly reflects what is actually measured, clarifies boundary conditions, and
prevents invalid assumptions about resource lifecycles.

## Examples

Bad:

```cpp
// Bad: Conflates having no pinner with being "unpinned", ignoring that pinners can dynamically allow deletion.
fUnpinnedCacheCount += strikePtr->fPinner == nullptr ? 1 : 0;
...
if (fUnpinnedCacheCount == 0 && !checkPinners) {
    return;
}
```

Good:

```cpp
// Good: Directly tracks pinner presence, making boundary checks and dynamic deletion policies clear.
fPinnerCount += strikePtr->fPinner != nullptr ? 1 : 0;
...
if (fPinnerCount == fCacheCount && !checkPinners) {
    return;
}
```
