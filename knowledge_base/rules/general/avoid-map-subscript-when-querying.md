---
name: skia/avoid-map-subscript-when-querying
description: When querying or conditionally updating associative containers like std::map or THashMap, use find() instead of operator[] to avoid unintended default insertions
rubric: correctness
severity: should_fix
---

**Rule.** Do not use operator[] to query or conditionally access elements in
associative containers (such as std::map, std::unordered_map, or
skia_private::THashMap) when the key might not exist and default insertion is
unintended. Use .find() instead to verify that the key exists before inspecting
or mutating mapped elements.

**Why.** In C++, evaluating operator[] on std::map, std::unordered_map, or
THashMap default-constructs and inserts a new entry when the key is absent.
Using operator[] for read-only lookups or conditional updates pollutes the
container with empty entries and can introduce subtle logic bugs or memory bloat
when inexistent keys should be ignored.

## Examples

Bad:

```cpp
void SlotManager::setTextStringSlot(const std::string& slotID, const SkString& text) {
    // Bad: operator[] default-constructs and inserts an entry if slotID is not found
    auto& handleGroup = fPropertyObserver->fTextMap[slotID];
    for (auto& handle : handleGroup) {
        handle->setText(text);
    }
}
```

Good:

```cpp
void SlotManager::setTextStringSlot(const std::string& slotID, const SkString& text) {
    // Good: find() checks for presence without modifying the map on missing keys
    const auto it = fPropertyObserver->fTextMap.find(slotID);
    if (it != fPropertyObserver->fTextMap.end()) {
        for (auto& handle : it->second) {
            handle->setText(text);
        }
    }
}
```
