---
name: skia/tools/omit-gpu-test-utils-in-tools
description: 'When writing or modifying testing and tooling code under tools/, omit redundant #if defined(GPU_TEST_UTILS) preprocessor guards around test-only headers, members, or options'
rubric: readability
severity: should_fix
---

**Rule.** Do not add `#if defined(GPU_TEST_UTILS)` or `#ifdef GPU_TEST_UTILS`
preprocessor guards around headers, fields, or helper calls in code under
`tools/`. Tooling infrastructure is exclusively compiled in test configurations
where test utilities are enabled, making preprocessor guards redundant.

**Why.** `GPU_TEST_UTILS` conditionally excludes test-only hooks and private
options from production builds of `src/gpu` and `include/gpu`. Code under
`tools/` is dedicated testing and tooling infrastructure that is never included
in production library builds, so wrapping test utilities or headers in
preprocessor guards adds unnecessary clutter.

## Examples

Bad:

```cpp
#if defined(GPU_TEST_UTILS)
#include "src/gpu/graphite/ContextOptionsPriv.h"
#endif

struct TestOptions {
    TestOptions() {
#if defined(GPU_TEST_UTILS)
        fContextOptions.fOptionsPriv = &fOptionsPriv;
#endif
    }

#if defined(GPU_TEST_UTILS)
    skgpu::graphite::ContextOptionsPriv fOptionsPriv;
#endif
};
```

Good:

```cpp
#include "src/gpu/graphite/ContextOptionsPriv.h"

struct TestOptions {
    TestOptions() {
        fContextOptions.fOptionsPriv = &fOptionsPriv;
    }

    skgpu::graphite::ContextOptionsPriv fOptionsPriv;
};
```
