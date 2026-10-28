---
name: skia/gpu/avoid-hardcoding-options-in-caps-init
description: When initializing GPU capabilities from ContextOptions or GrContextOptions in Caps, do not unconditionally hardcode feature flags to true after reading options
rubric: correctness
severity: blocking
---

**Rule.** When initializing GPU capabilities and feature flags from
ContextOptions or GrContextOptions (such as in Caps::finishInitialization), do
not unconditionally assign hardcoded boolean values to capability flags after
reading them from options. Ensure caller-provided options and test overrides
(such as options.fOptionsPriv) take effect and can toggle or disable the
feature.

**Why.** Hardcoding capability flags unconditionally overrides caller-configured
options and test overrides (often inadvertently left over from local debugging),
preventing tests from exercising fallback execution paths or verifying behavior
when the capability is disabled.

## Examples

Bad:

```cpp
#if defined(GPU_TEST_UTILS)
    if (options.fOptionsPriv) {
        fDrawListLayer = options.fOptionsPriv->fDrawListLayer;
    }
#endif
    // Hardcoded assignment unconditionally overrides caller options.
    fDrawListLayer = true;
```

Good:

```cpp
#if defined(GPU_TEST_UTILS)
    if (options.fOptionsPriv) {
        fDrawListLayer = options.fOptionsPriv->fDrawListLayer;
    }
#endif
    fDrawListLayer |= options.fUseDrawListLayer;
```
