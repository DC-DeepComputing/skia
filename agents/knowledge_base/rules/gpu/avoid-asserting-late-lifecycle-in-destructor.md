---
name: skia/gpu/avoid-asserting-late-lifecycle-in-destructor
description: When implementing destructors for multi-stage configuration helpers, do not assert completion of late-stage execution steps if callers can legitimately bail out early
rubric: correctness
severity: should_fix
---

**Rule.** Do not assert in a helper object's destructor that late-stage
execution, finalization, or enabling steps were invoked if callers can
legitimately abort or bail out after an intermediate inspection or query stage.
If unexecuted final steps warrant developer awareness, log a warning via
SkDebugf instead of crashing with SkASSERT.

**Why.** Callers often query backend capabilities, device extensions, or
hardware support during intermediate lifecycle stages and may legitimately bail
out before proceeding to the final enablement step. Enforcing completion of all
lifecycle stages via SkASSERT in the destructor causes false-positive test
crashes and breaks valid early-exit error handling paths.

## Examples

Bad:

```cpp
VulkanPreferredFeatures::~VulkanPreferredFeatures() {
    // Bad: asserts final enabling step, causing crashes if caller bailed out after query
    SkASSERT(fHasAddedToInstanceExtensions && fHasAddedFeaturesToQuery && fHasAddedFeaturesToEnable);
}
```

Good:

```cpp
VulkanPreferredFeatures::~VulkanPreferredFeatures() {
    // Good: log a warning for missed final step, but do not assert failure on early bailout
    if (!fHasAddedFeaturesToEnable) {
        SkDebugf("WARNING: VulkanPreferredFeatures::addFeaturesToEnable() was not called\n");
    }
    SkASSERT(fHasAddedToInstanceExtensions && fHasAddedFeaturesToQuery);
}
```
