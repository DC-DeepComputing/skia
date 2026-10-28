---
name: skia/avoid-associative-container-at-lookup
description: When querying associative containers like std::map or std::unordered_map, use find() instead of at() to avoid process crashes from disabled exceptions
rubric: correctness
severity: should_fix
---

**Rule.** Do not use .at() to look up keys in associative containers (such as
std::map or std::unordered_map) when the key might be absent. Because Skia
compiles with C++ exceptions disabled (-fno-exceptions), missing keys cause
.at() to invoke std::terminate() and crash the process rather than throw. Use
.find() (or .contains() for presence checks) and verify the returned iterator or
pointer against end() before accessing the value.

**Why.** std::map::at() and std::unordered_map::at() throw std::out_of_range
when a queried key does not exist. In builds compiled with -fno-exceptions,
throwing an exception triggers an uncatchable process abort via
std::terminate(). Using find() safely inspects key presence and allows graceful
fallback handling without crashing.

## Examples

Bad:

```cpp
// Bad: .at() throws std::out_of_range on missing keys, terminating the process under -fno-exceptions
sk_sp<skresources::ImageAsset> loadImageAsset(const char slot_name[]) const override {
    return fImageAssetMap.at(slot_name);
}
```

Good:

```cpp
// Good: find() checks against end() and handles missing keys gracefully
sk_sp<skresources::ImageAsset> loadImageAsset(const char slot_name[]) const override {
    const auto it = fImageAssetMap.find(slot_name);
    return it == fImageAssetMap.end() ? nullptr : it->second;
}
```
