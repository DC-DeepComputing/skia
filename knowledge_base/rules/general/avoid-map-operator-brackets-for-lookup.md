---
name: skia/avoid-map-operator-brackets-for-lookup
description: When reading from associative containers like std::map or THashMap, use find() instead of operator[] to avoid unintentional insertion of default values on missing keys
rubric: correctness
severity: should_fix
---

**Rule.** Do not use operator[] on associative containers (such as std::map,
std::unordered_map, or skia_private::THashMap) for read-only lookups or
conditional updates where the key may not exist. Use find() to query the key and
check against end() (or nullptr) to retrieve the element without mutating the
map.

**Why.** operator[] default-constructs and inserts a new element into the map
whenever the queried key does not already exist. When querying or updating
values only if a key is present, using operator[] mutates the container,
increases its size, and pollutes it with empty or uninitialized entries, which
can alter container traversal and break subsequent size or presence checks.

## Examples

Bad:

```cpp
void SlotManager::setOpacitySlot(std::string slotID, SkScalar opacity) {
    // Bad: operator[] inserts an empty vector if slotID is missing
    auto& handleGroup = fPropertyObserver->fOpacityMap[slotID];
    for (auto& handle : handleGroup) {
        handle->set(opacity);
    }
}
```

Good:

```cpp
void SlotManager::setOpacitySlot(std::string slotID, SkScalar opacity) {
    // Good: find() checks existence without mutating the map
    const auto it = fPropertyObserver->fOpacityMap.find(slotID);
    if (it != fPropertyObserver->fOpacityMap.end()) {
        for (auto& handle : it->second) {
            handle->set(opacity);
        }
    }
}
```
