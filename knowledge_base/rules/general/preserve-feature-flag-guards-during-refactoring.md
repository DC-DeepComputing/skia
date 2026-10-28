---
name: skia/preserve-feature-flag-guards-during-refactoring
description: When adding algorithm passes or refactoring code governed by client compile-time flags, preserve preprocessor guards and flag-specific fallback handling
rubric: correctness
severity: should_fix
---

**Rule.** When introducing new algorithm passes or refactoring code in
subsystems governed by compile-time client flags (such as SK_AVOID_*,
SK_ENABLE_*, or SK_DISABLE_*), ensure new execution paths are guarded (e.g.,
#ifndef SK_AVOID_...) and all existing conditional preprocessor blocks are
preserved in extracted routines. Maintain flag-specific branches—such as
identity passes, fallback execution paths, or custom buffer initialization—so
embedders disabling specialized paths continue to use expected fallbacks without
uninitialized buffers or performance regressions.

**Why.** Alternative build configurations and embedder feature flags (such as
Chromium bypassing slow raster pipeline blurs) alter algorithm selection and
execution paths. Omitting preprocessor guards on new passes or dropping
conditional blocks during refactoring silently compiles in default CI where
flags are unset, but breaks embedders by violating avoidance toggles or
triggering crashes and uninitialized buffers.

## Examples

Bad:

```cpp
// 1. Introducing a new pass without respecting existing avoidance flags:
auto makeMaker = [&](float sigma) -> PassMaker* {
    if (PassMaker* maker = GaussianPass<uint32_t>::MakeMaker(sigma, &alloc)) {
        return maker;
    }
    if (PassMaker* maker = ThreeBoxApproxPass::MakeMaker(sigma, &alloc)) {
        return maker;
    }
    ...
};

// 2. Refactoring without preserving flag-specific fallback/identity handling:
if (makerY->window() > 1) { ... }
// Missing: #if defined(SK_AVOID_SLOW_RASTER_PIPELINE_BLURS) identity handling
dstBounds = originalDstBounds.makeOffset(-dstOrigin);
return SkSpecialImages::MakeFromRaster(dstBounds, dst, SkSurfaceProps{});
```

Good:

```cpp
// 1. Guard new algorithm passes so embedder avoidance flags are respected:
auto makeMaker = [&](float sigma) -> PassMaker* {
#ifndef SK_AVOID_SLOW_RASTER_PIPELINE_BLURS
    if (PassMaker* maker = GaussianPass<uint32_t>::MakeMaker(sigma, &alloc)) {
        return maker;
    }
#endif
    if (PassMaker* maker = ThreeBoxApproxPass::MakeMaker(sigma, &alloc)) {
        return maker;
    }
    ...
};

// 2. Preserve flag-specific fallback and identity handling when refactoring:
if (makerY->window() > 1) { ... }
#if defined(SK_AVOID_SLOW_RASTER_PIPELINE_BLURS)
if (makerX->window() == 1 && makerY->window() == 1) {
    dst.writePixels(src.pixmap(), srcBounds.left() - dstBounds.left(),
                    srcBounds.top() - dstBounds.top());
}
#endif
dstBounds = originalDstBounds.makeOffset(-dstOrigin);
return SkSpecialImages::MakeFromRaster(dstBounds, dst, SkSurfaceProps{});
```
