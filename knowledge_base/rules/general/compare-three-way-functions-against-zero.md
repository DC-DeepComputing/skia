---
name: skia/compare-three-way-functions-against-zero
description: When testing equality or inequality with three-way comparison functions like memcmp or strcmp, explicitly compare against zero (== 0) instead of boolean negation (!memcmp)
rubric: readability
severity: nit
---

**Rule.** When testing for equality or difference using three-way comparison
functions (such as memcmp, strcmp, or strncmp), explicitly compare the result
against zero (e.g., memcmp(...) == 0 or memcmp(...) != 0). Do not use boolean
negation (!memcmp(...)) to test for matching buffers or strings.

**Why.** Three-way comparison functions return 0 when buffers match and non-zero
when they differ. Using boolean negation (!memcmp(...)) counter-intuitively
reads like checking for inequality, introducing cognitive confusion and
readability hazards during review and maintenance.

## Examples

Bad:

```cpp
if (cacheHeader[2] == physDevProps.vendorID &&
    cacheHeader[3] == physDevProps.deviceID &&
    !memcmp(&cacheHeader[4], supportedPipelineCacheUUID, VK_UUID_SIZE)) {
    return true;
}
```

Good:

```cpp
if (cacheHeader[2] == physDevProps.vendorID &&
    cacheHeader[3] == physDevProps.deviceID &&
    memcmp(&cacheHeader[4], supportedPipelineCacheUUID, VK_UUID_SIZE) == 0) {
    return true;
}
```
