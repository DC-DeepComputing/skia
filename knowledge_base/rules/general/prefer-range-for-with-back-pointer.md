---
name: skia/prefer-range-for-with-back-pointer
description: When iterating contiguous containers or spans and checking for the last element, prefer range-for loops comparing element addresses to &container.back() over index tracking
rubric: readability
severity: nit
---

**Rule.** Prefer range-based for loops over index-based loops when iterating
contiguous containers or spans (such as SkSpan, std::vector, or
skia_private::TArray). When an index is only used to identify whether the
current element is the last item, compare the element address directly with
&container.back() (`&elem == &container.back()`).

**Why.** Range-based for loops eliminate manual index maintenance, bounds
boilerplate, and off-by-one errors. For contiguous sequences that are guaranteed
non-empty, pointer comparison against the address of the final element expresses
last-element checks concisely without sacrificing range iteration.

## Examples

Bad:

```cpp
for (size_t i = 0; i < verbs.size(); ++i) {
    switch (verbs[i]) {
        case SkPathVerb::kMove:
            if (++moveCount > 1) {
                return false;
            }
            break;
        case SkPathVerb::kClose:
            return (i + 1) == verbs.size();
        default:
            break;
    }
}
```

Good:

```cpp
for (const auto& verb : verbs) {
    switch (verb) {
        case SkPathVerb::kMove:
            if (++moveCount > 1) {
                return false;
            }
            break;
        case SkPathVerb::kClose:
            return &verb == &verbs.back();
        default:
            break;
    }
}
```
