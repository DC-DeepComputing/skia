---
name: skia/update-conditionally-compiled-call-sites
description: When refactoring, moving, or writing code inside conditionally compiled preprocessor blocks, verify call sites and ensure all required header dependencies are explicitly included
rubric: correctness
severity: blocking
---

**Rule.** When refactoring, renaming, or encapsulating APIs, types, or member
variables, verify and update all call sites across conditionally compiled
preprocessor blocks throughout the codebase. Additionally, when moving or
writing code inside conditional blocks guarded by platform, feature, tracing, or
SDK macros (such as SK_BUILD_FOR_ANDROID_FRAMEWORK, SK_TRACE_*, SK_DEBUG,
GPU_TEST_UTILS, or DWRITE_CORE), ensure all header files required by the guarded
code paths (such as SkColorSpace.h or SkBitmap.h) are explicitly included. Do
not rely on local host builds or standard CQ bots to catch issues in inactive
branches.

**Why.** Standard developer builds and baseline CQ test configurations often
compile with optional tracing, debug flags, platform-specific targets, or
advanced SDK versions disabled. Overlooked call sites or missing header
dependencies inside inactive conditional blocks do not trigger compiler errors
in standard CI, but break specialized builders, downstream rolls (such as
Android or Chromium), or builds against newer platform SDKs.

## Examples

Bad:

```cpp
// 1. Stale call sites in inactive conditional blocks during refactoring:
#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    drawPass->fPipelineDrawAreas[key.pipelineIndex()] +=
            draw.fDrawParams.clip().drawBounds().area();  // Stale member access
#endif

// 2. Missing required header dependencies when moving guarded code to a new file:
// (Omits #include "include/core/SkColorSpace.h")
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior) {
    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
}
#endif
```

Good:

```cpp
// 1. Updated call sites inside conditionally compiled tracing blocks:
#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    drawPass->fPipelineDrawAreas[key.pipelineIndex()] +=
            draw.drawParams().clip().drawBounds().area();
#endif

// 2. Explicitly include all header dependencies required by guarded code:
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior) {
    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
}
#endif
```
