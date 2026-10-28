---
name: skia/check-container-emptiness-after-lookup
description: When accessing elements from a container retrieved via map or collection lookup, check !container.empty() in addition to lookup success before indexing or calling front()
rubric: correctness
severity: should_fix
---

**Rule.** When retrieving a container from an associative lookup (such as
map.find()), verify that the container is non-empty (!container.empty()) in
addition to checking that the key was found before accessing elements by index,
at(), or front(). Do not assume a successful key lookup guarantees the mapped
container holds elements.

**Why.** A successful map lookup only verifies that a key is present; the mapped
container may be empty. Accessing element 0 via operator[], .at(0), or .front()
on an empty container causes out-of-bounds assertions, std::out_of_range
exceptions, or crashes.

## Examples

Bad:

```cpp
const auto valueGroup = fColorMap.find(slotID);
// Bug: valueGroup is non-null if key exists, but the mapped container may be empty.
return valueGroup ? *(valueGroup->at(0).value) : SK_ColorBLACK;
```

Good:

```cpp
const auto valueGroup = fColorMap.find(slotID);
// Verify both key lookup success and container non-emptiness before indexing element 0.
return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : SK_ColorBLACK;
```
