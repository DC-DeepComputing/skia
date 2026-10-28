---
name: skia/gpu/qualify-property-and-limit-variables
description: When declaring backend feature, property, or limit variables and wrapper fields, qualify identifiers by entity to prevent repetitive access chains and ambiguous names
rubric: readability
severity: nit
---

**Rule.** When declaring local variables, parameters, or member fields for
backend feature, property, and limit structures (such as Vulkan or Dawn
features, properties, and limits), qualify the identifier with the specific
entity being queried or represented (e.g., 'deviceFeatures', 'testVkFeatures',
'deviceProperties', 'deviceLimits', 'formatProperties') rather than using bare,
generic names like 'features', 'properties', or 'limits'. In wrapper structs and
local scopes, ensure outer variables and fields do not shadow or repeat inner
member names, avoiding repetitive chained accesses like
'features.features.features'.

**Why.** GPU capability initialization queries multiple distinct categories of
hardware and driver characteristics, including physical device, driver, memory,
and format properties or feature sets. Qualifying variable and field names
prevents ambiguity over which graphics API structure or entity is inspected,
clarifies abstraction layering, and eliminates confusing, repetitive member
access chains.

## Examples

Bad:

```cpp
// Repetitive member chain from generic wrapper and parameter naming:
features.features.features.robustBufferAccess = VK_FALSE;

// Bare generic property and limit names:
PhysicalDeviceProperties properties;
this->getProperties(vkInterface, physDev, &properties);
const VkPhysicalDeviceLimits& limits = properties.base.properties.limits;
```

Good:

```cpp
// Disambiguated wrapper field and outer variable names:
testVkFeatures.deviceFeatures.features.robustBufferAccess = VK_FALSE;

// Qualified by specific entity type:
PhysicalDeviceProperties deviceProperties;
this->getProperties(vkInterface, physDev, &deviceProperties);
const VkPhysicalDeviceLimits& deviceLimits = deviceProperties.base.properties.limits;
```
