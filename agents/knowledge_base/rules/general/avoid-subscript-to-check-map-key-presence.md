---
name: skia/avoid-subscript-to-check-map-key-presence
description: When querying associative containers or maps for key presence, use find() or contains() instead of operator[] to avoid inserting default-constructed entries
rubric: correctness
severity: should_fix
---

**Rule.** Do not use operator[] to test for key presence or conditionally
inspect values in associative containers (such as std::map, std::unordered_map,
or skia_private::THashMap). Use find() (checking against end() or null) or
contains() instead, avoiding accidental insertion of default-constructed entries
on key misses.

**Why.** In standard C++ associative containers (std::map, std::unordered_map)
and skia_private::THashMap, operator[] inserts a default-constructed value
whenever the key is not found. Using operator[] for presence checks or read-only
access mutates the container, introducing unexpected dummy entries, increasing
memory usage, and corrupting subsequent iterations or size checks.

## Examples

Bad:

```cpp
void SlotManager::setImageSlot(std::string slotID, sk_sp<skresources::ImageAsset> img) {
    // Bad: operator[] inserts a default-constructed entry if slotID is missing.
    if (auto assetProxy = fResourceProvider->fImageAssetMap[slotID]) {
        assetProxy->setImageAsset(std::move(img));
    }
}
```

Good:

```cpp
void SlotManager::setImageSlot(std::string slotID, sk_sp<skresources::ImageAsset> img) {
    // Good: find() inspects presence without mutating the map.
    const auto it = fResourceProvider->fImageAssetMap.find(slotID);
    if (it != fResourceProvider->fImageAssetMap.end() && it->second) {
        it->second->setImageAsset(std::move(img));
    }
}
```
