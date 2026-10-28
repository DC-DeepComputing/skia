---
name: skia/api/prefer-visitor-callbacks-over-intermediate-containers
description: When querying or iterating elements across APIs, accept or pass visitor callbacks instead of allocating and populating intermediate heap containers like std::map or std::vector
rubric: safety
severity: should_fix
---

**Rule.** When designing or consuming iteration and inspection queries, accept
or pass a visitor or streaming callback function rather than allocating,
populating, and returning intermediate heap containers (such as std::map,
std::vector, or THashMap). Stream elements directly to caller callbacks with
zero heap allocations when callers only need to iterate over or process query
results.

**Why.** Populating intermediate heap-allocating containers (especially
node-allocating collections like std::map, which call operator new for every
item) introduces unnecessary dynamic memory churn and extra copy/traversal
passes. Streaming items directly to inline visitor callbacks eliminates
intermediate allocations and preserves cache locality.

## Examples

Bad:

```cpp
// Bad: Allocates a node in std::map on the heap for every element just to iterate over it
std::map<SkUnicode::Position, bool> spaces;
if (!fUnicode->getSpaces(fText.c_str(), fText.size(), &spaces)) {
    return false;
}
for (auto space : spaces) {
    fCodeUnitProperties[space.first] |= space.second ? kSpaceFlag : 0;
}
```

Good:

```cpp
// Good: Visitor callback streams elements directly with zero dynamic heap allocations
fUnicode->forEachBreak(fText.c_str(), fText.size(),
    [this](size_t start, size_t end, SkUnicode::BreakType type) {
        if (type == SkUnicode::BreakType::kSpaces) {
            for (size_t i = start; i < end; ++i) {
                fCodeUnitProperties[i] |= CodeUnitFlags::kPartOfWhiteSpaceBreak;
            }
        }
        return true;
    });
```
