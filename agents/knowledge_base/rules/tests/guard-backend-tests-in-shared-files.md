---
name: skia/tests/guard-backend-tests-in-shared-files
description: In shared test and GM files, guard backend definitions with canonical SK_GANESH or SK_GRAPHITE, and preserve non-GPU fallbacks for CPU configurations
rubric: correctness
severity: blocking
---

**Rule.** In shared test files, GMs, and tool utilities outside dedicated
backend folders, guard backend-specific test definitions (`DEF_GRAPHITE_TEST_*`,
`DEF_GANESH_TEST_*`, `DEF_SIMPLE_GPU_GM*`), helper constructors, methods, and
types (`GrDirectContext` or Graphite contexts) with `#if defined(SK_GRAPHITE)`
or `#if defined(SK_GANESH)`. Always use canonical `SK_GRAPHITE` and `SK_GANESH`
macros rather than unprefixed names. Additionally, preserve non-GPU fallback
paths and keep tests in general source lists rather than returning null or
moving files to GPU-only lists (`gpu_gm_sources`) when the test can still
execute in CPU-only configurations.

**Why.** Shared test files, GMs, and tool utilities are compiled across diverse
build configurations, including CPU-only builds or configurations where either
Ganesh or Graphite is disabled. Omitting preprocessor guards causes build or
link failures when backend types are omitted, using unprefixed macros like
GANESH silently skips tests, and discarding non-GPU fallbacks loses test
coverage in CPU-only configurations.

## Examples

Bad:

```cpp
// tests/ or gm/: missing guard or using unprefixed macro
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BlurTest, r, ctx, CtsEnforcement::kNever) { ... }
#if defined(GANESH)
DEF_SIMPLE_GPU_GM(Bitmaptiled, rContext, canvas, 100, 100) { ... }
#endif

// Eliminating non-GPU fallback in shared GM/test:
#if defined(SK_GRAPHITE)
if (recorder) { return lazyYUVImage->refImage(recorder, type); }
#endif
#if defined(SK_GANESH)
if (direct) { return lazyYUVImage->refImage(direct, type); }
#endif
return nullptr; // Breaks CPU-only execution
```

Good:

```cpp
// tests/ or gm/: guarded with canonical SK_GRAPHITE / SK_GANESH
#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BlurTest, r, ctx, CtsEnforcement::kNever) { ... }
#endif
#if defined(SK_GANESH)
DEF_SIMPLE_GPU_GM(Bitmaptiled, rContext, canvas, 100, 100) { ... }
#endif

// Preserving non-GPU fallback:
#if defined(SK_GRAPHITE)
if (recorder) { return lazyYUVImage->refImage(recorder, type); } else
#endif
{ return lazyYUVImage->refImage(direct, type); }
```
