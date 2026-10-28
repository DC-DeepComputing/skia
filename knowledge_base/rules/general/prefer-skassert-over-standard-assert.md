---
name: skia/prefer-skassert-over-standard-assert
description: When asserting conditions or invariants, use Skia's SkASSERT or SkASSERT_RELEASE macros instead of standard C/C++ assert to ensure failure routing via SK_ABORT
rubric: readability
severity: should_fix
---

**Rule.** Do not use standard C/C++ assert() or include <cassert> / <assert.h>;
use Skia's assertion macros such as SkASSERT or SkASSERT_RELEASE instead. Ensure
assertion failures route through Skia's failure mechanisms so embedders can
handle diagnostics and crashes consistently.

**Why.** Skia's assertion macros route failures through SK_ABORT, which
downstream embedders (such as Chromium and Android) hook to collect crash
telemetry, symbolicate stack traces, and handle fatal errors. Calling standard
C/C++ assert() bypasses embedder hooks and behaves inconsistently across build
configurations.

## Examples

Bad:

```cpp
#include <cassert>

assert(!mx.neverRequestPath || !mx.computeFromPath);
```

Good:

```cpp
#include "include/core/SkTypes.h"

SkASSERT(!mx.neverRequestPath || !mx.computeFromPath);
```
