---
name: skia/tests/gate-assertions-on-capabilities-not-backend
description: In GPU tests, gate conditional assertions or test paths on specific Caps capability queries rather than blanket BackendApi checks
rubric: testing
severity: should_fix
---

**Rule.** In GPU unit tests, do not guard behavioral assertions or test
execution paths on blanket backend checks (e.g. context->backend() !=
BackendApi::kVulkan). Query specific capability methods on Caps (e.g.
caps->supportsHostImageCopy()) so configurations and devices of that backend
lacking the optional feature still execute and validate the default path.

**Why.** Blanket backend checks disable test assertions across all
configurations of that backend, forfeiting coverage on devices where the
optional feature or extension is disabled or unsupported. Querying specific
capability flags via Caps ensures that configurations lacking the capability
continue exercising and asserting standard fallback behavior.

## Examples

Bad:

```cpp
// Blanket backend check disables assertion even when host image copy is unsupported on Vulkan
if (context->backend() != BackendApi::kVulkan) {
    REPORTER_ASSERT(reporter, !finishCtx.fFinishedUpdate);
}
```

Good:

```cpp
// Specific capability query asserts standard behavior on any context lacking the feature
if (!caps->supportsHostImageCopy()) {
    REPORTER_ASSERT(reporter, !finishCtx.fFinishedUpdate);
}
```
