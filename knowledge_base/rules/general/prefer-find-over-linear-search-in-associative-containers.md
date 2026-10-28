---
name: skia/prefer-find-over-linear-search-in-associative-containers
description: When querying or matching keys in associative containers like std::map or THashMap, use find() instead of linearly iterating over entries in a loop
rubric: safety
severity: should_fix
---

**Rule.** When querying or matching keys in associative containers (such as
std::map, std::unordered_map, or skia_private::THashMap), use member lookup
methods like find() or contains() instead of linearly iterating over the
container in a for loop. Check the returned iterator or pointer before accessing
mapped values.

**Why.** Associative containers provide O(1) or O(log N) key lookups. Scanning
the container sequentially with a loop turns a fast key query into an O(N)
linear search, introducing quadratic runtime overhead when executed inside
callbacks or loops while adding unnecessary iteration boilerplate.

## Examples

Bad:

```cpp
// Bad: linearly scanning an associative container to match a key
for (auto& entry : fColorMap) {
    if (entry.first == node_name) {
        c()->set(entry.second);
    }
}
```

Good:

```cpp
// Good: use find() for direct O(1) / O(log N) key lookup
const auto it = fColorMap.find(node_name);
if (it != fColorMap.end()) {
    c()->set(it->second);
}
```
