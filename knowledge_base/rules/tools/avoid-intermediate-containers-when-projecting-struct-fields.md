---
name: skia/tools/avoid-intermediate-containers-when-projecting-struct-fields
description: When querying and filtering driver or subsystem records like Vulkan extensions, populate target attribute containers directly instead of staging full structs into intermediate containers
rubric: safety
severity: should_fix
---

**Rule.** When collecting or filtering driver metadata or multi-field records
(such as Vulkan extension or layer properties) where callers only require a
specific attribute (such as extension names), populate or return the target
container directly. Do not accumulate full structs into an intermediate
container only to immediately iterate over them and extract a single member
field.

**Why.** Staging multi-field structs into an intermediate container before
projecting a single attribute incurs redundant heap allocations, unnecessary
struct copying, and an extra iteration pass. Populating the caller's target
container directly avoids intermediate allocator churn and eliminates
boilerplate projection loops.

## Examples

Bad:

```cpp
// BAD: Staging full structs in an intermediate array only to extract extension names
TArray<VkExtensionProperties> instExtsToEnable;
init_instance_extensions_and_layers(..., instExtsToEnable);

std::vector<const char*> enabledInstExtNames;
for (const auto& ext : instExtsToEnable) {
    enabledInstExtNames.push_back(ext.extensionName);
}
```

Good:

```cpp
// GOOD: Helper directly populates the target attribute container during filtering
std::vector<const char*> enabledInstExtNames;
init_instance_extensions_and_layers(..., enabledInstExtNames);
```
