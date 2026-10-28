---
name: skia/tests/omit-client-migration-guards-in-tests
description: When updating unit tests under tests/ during API migrations, omit conditional compilation guards for client staging flags like SK_IGNORE_* or SK_SUPPORT_LEGACY_*
rubric: correctness
severity: should_fix
---

**Rule.** Do not add '#if !defined(...)' or '#ifdef' conditional compilation
guards for client migration or staging flags (such as SK_IGNORE_* or
SK_SUPPORT_LEGACY_*) in Skia unit test files ('tests/**'). Target the latest
Skia API directly in test code, as downstream clients like Chromium only compile
Skia library targets and do not build Skia's internal test suite.

**Why.** Temporary staging flags decouple Skia API modifications in downstream
client repositories during rolls. Because downstream embedders never compile
Skia's internal unit tests, wrapping test code in client migration flags is
redundant, clutters tests with dead preprocessor branches, and increases
maintenance overhead.

## Examples

Bad:

```cpp
RenderPassProperties bgra_1_depth() {
    return { DepthStencilFlags::kDepth,
             kBGRA_8888_SkColorType,
#if !defined(SK_IGNORE_RENDER_PASS_PROPERTIES_COLOR_SPACE)
             /* dstColorSpace= */ nullptr,
#endif
             /* requiresMSAA= */ false };
}
```

Good:

```cpp
RenderPassProperties bgra_1_depth() {
    return { DepthStencilFlags::kDepth,
             kBGRA_8888_SkColorType,
             /* dstColorSpace= */ nullptr,
             /* requiresMSAA= */ false };
}
```
