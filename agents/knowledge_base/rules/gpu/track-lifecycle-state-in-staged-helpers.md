---
name: skia/gpu/track-lifecycle-state-in-staged-helpers
description: When implementing multi-step configuration or staged initialization helper classes, track caller lifecycle state with internal boolean flags and assert or warn against out-of-order calls
rubric: correctness
severity: should_fix
---

**Rule.** In multi-step configuration or staged initialization helper classes
where methods have ordering dependencies, track caller lifecycle state using
internal boolean flags. In subsequent stages and the destructor, validate
execution sequence by asserting expected prerequisites with SkASSERT and
emitting a warning with SkDebugf if required steps were omitted or invoked out
of order.

**Why.** Complex staged workflows (such as querying hardware features before
enabling them) depend on callers executing steps in sequence. Without internal
lifecycle tracking, skipped or out-of-order invocations fail silently, resulting
in disabled features, subtle initialization defects, or un-optimized fallback
configurations.

## Examples

Bad:

```cpp
void VulkanPreferredFeatures::addFeaturesToEnable(
        std::vector<const char*>& appExtensions,
        VkPhysicalDeviceFeatures2& appFeatures) {
    // BAD: Assumes addFeaturesToQuery() was called without verifying prerequisite state
    FeaturesToAdd toAdd;
    this->collectFeatures(appExtensions, appFeatures, &toAdd);
}
```

Good:

```cpp
void VulkanPreferredFeatures::addFeaturesToEnable(
        std::vector<const char*>& appExtensions,
        VkPhysicalDeviceFeatures2& appFeatures) {
    // GOOD: Validate prerequisite steps using internal lifecycle flags
    if (!fHasAddedFeaturesToQuery) {
        SkDebugf("WARNING: addFeaturesToQuery() was not called before addFeaturesToEnable()\n");
    }
    SkASSERT(fHasAddedFeaturesToQuery);
    fHasAddedFeaturesToEnable = true;
    FeaturesToAdd toAdd;
    this->collectFeatures(appExtensions, appFeatures, &toAdd);
}
```
