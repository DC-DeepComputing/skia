---
name: skia/avoid-unintended-map-insertion-via-operator-subscript
description: When querying or accessing values in associative containers like std::map or std::unordered_map, use find() instead of operator[] to prevent unintended insertion of default elements
rubric: correctness
severity: should_fix
---

**Rule.** When querying or accessing elements in associative containers (such as
std::map or std::unordered_map) where the key might not exist, do not use
operator[]. Use find() and verify the returned iterator or pointer against end()
(or nullptr) before accessing the value, or check contains() when only presence
is tested.

**Why.** In associative containers like std::map and std::unordered_map,
operator[] default-constructs and inserts an entry if the key is not present.
Using operator[] to look up or update an entry that may not exist inadvertently
mutates the container, introducing unwanted empty elements, triggering
unnecessary allocations, and potentially invalidating iterators or references.

## Examples

Bad:

```cpp
// Bad: operator[] inserts a default-constructed entry if slotID is absent
void SlotManager::setColorSlot(const std::string& slotID, SkColor color) {
    auto& handleGroup = fPropertyObserver->fColorMap[slotID];
    for (auto& handle : handleGroup) {
        handle->set(color);
    }
}
```

Good:

```cpp
// Good: find() checks key existence without mutating the container
void SlotManager::setColorSlot(const std::string& slotID, SkColor color) {
    const auto it = fPropertyObserver->fColorMap.find(slotID);
    if (it != fPropertyObserver->fColorMap.end()) {
        for (auto& handle : it->second) {
            handle->set(color);
        }
    }
}
```
