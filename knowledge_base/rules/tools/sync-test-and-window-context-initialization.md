---
name: skia/tools/sync-test-and-window-context-initialization
description: When updating GPU backend context initialization or third-party API descriptors in Skia tools, update both test harnesses and window contexts
rubric: correctness
severity: should_fix
---

**Rule.** When adapting Skia tools to upstream third-party API changes, breaking
changes, or migration macros (such as Dawn InstanceDescriptor or Vulkan instance
initialization), update all counterpart instance- and device-creation sites
across both test harnesses (tools/graphite/, tools/ganesh/) and window/viewer
contexts (tools/window/). Ensure conditional compilation guards and descriptor
configurations remain consistent across both environments.

**Why.** Backend instance and device setup in Skia tools is partitioned between
headless test harnesses (used by DM) and interactive window contexts (used by
Viewer). Updating upstream API breaking changes or staging macros in only test
contexts leaves window contexts broken when third-party dependencies roll in CI.

## Examples

Bad:

```cpp
// In tools/window/GraphiteDawnWindowContext.cpp:
// Missed updating to match upstream Dawn breaking change applied in GraphiteDawnTestContext.cpp
desc.capabilities.timedWaitAnyEnable = true;
```

Good:

```cpp
// In tools/window/GraphiteDawnWindowContext.cpp:
#if defined(WGPU_BREAKING_CHANGE_INSTANCE_FEATURES_LIMITS)
    static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    desc.requiredFeatureCount = 1;
    desc.requiredFeatures = &kTimedWaitAny;
#else
    desc.capabilities.timedWaitAnyEnable = true;
#endif
```
