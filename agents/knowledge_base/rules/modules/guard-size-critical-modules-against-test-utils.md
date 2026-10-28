---
name: skia/modules/guard-size-critical-modules-against-test-utils
description: 'When maintaining size-critical production modules like CanvasKit, retain compile-time #error canary checks to prevent accidental inclusion of test utilities'
rubric: safety
severity: nit
---

**Rule.** In size-critical production modules and WebAssembly bindings (such as
CanvasKit), retain or add compile-time preprocessor canary checks (#if
defined(...) #error ... #endif) against test-only flags (such as GPU_TEST_UTILS
or GR_TEST_UTILS). Ensure builds fail immediately if test utilities are
inadvertently enabled in production configurations.

**Why.** Test utilities pull in extensive testing harnesses, mock
implementations, and verification data structures that significantly bloat
binary size. In strictly size-constrained targets like WebAssembly and
CanvasKit, compile-time canary checks prevent accidental configuration
regressions before deployment.

## Examples

Bad:

```cpp
// modules/canvaskit/canvaskit_bindings.cpp
// Test utility macro was removed or left unguarded, risking codesize bloat if enabled:
#include "include/gpu/ganesh/GrDirectContext.h"
```

Good:

```cpp
// modules/canvaskit/canvaskit_bindings.cpp
#if defined(GPU_TEST_UTILS) || defined(GR_TEST_UTILS)
#error "This define should not be set, as it brings in test-only things and bloats codesize."
#endif
```
