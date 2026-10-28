---
name: skia/prefer-tarray-over-std-vector
description: When defining internal collections or data structures, prefer Skia containers (TArray, THashMap, SkString) and structs over STL types (std::vector, std::unordered_map, std::pair)
rubric: safety
severity: should_fix
---

**Rule.** In internal Skia and module code, prefer Skia containers and string
types over standard library equivalents (such as skia_private::TArray or
STArray<N, T> over std::vector, skia_private::THashMap over std::unordered_map,
SkString over std::string, and dedicated structs over std::pair) to prevent
binary size bloat. For bounded or local collections, use STArray<N, T> to
provide inline stack storage and eliminate heap allocations. When wrapping
dynamic arrays from C-style APIs, size TArray with resize_back() or
reserve_exact() and pass .data() instead of using std::vector.

**Why.** STL containers and types (std::vector, std::unordered_map, std::string,
std::pair) generate larger template instantiations, header overhead, and binary
footprint than Skia's internal containers. skia_private::TArray and THashMap are
optimized for Skia's memory layout, leverage sk_is_trivially_relocatable
optimizations during resizing, and STArray<N, T> avoids dynamic heap allocations
for small collections.

## Examples

Bad:

```cpp
// Container and associative map definitions:
using SlotID = std::string;
template <typename T>
using ValuePair = std::pair<T, sk_sp<sksg::Node>>;
template <typename T>
using SlotMap = std::unordered_map<SlotID, std::vector<ValuePair<T>>>;

// Dynamic arrays or C-style query APIs:
#include <vector>
std::vector<VkQueueFamilyProperties> queueProps(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, queueProps.data());

// Small local collections:
std::vector<const Expression*> nextExprs;
```

Good:

```cpp
// Container and associative map definitions:
using SlotID = SkString;
template <typename T>
struct ValuePair {
    T value;
    sk_sp<sksg::Node> node;
};
template <typename T>
using SlotMap = skia_private::THashMap<SlotID, skia_private::TArray<ValuePair<T>>>;

// Dynamic arrays or C-style query APIs:
#include "include/private/base/SkTArray.h"
skia_private::TArray<VkQueueFamilyProperties> queueProps;
queueProps.resize_back(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, queueProps.data());

// Small local collections with inline stack storage:
skia_private::STArray<1, const Expression*> nextExprs;
```
