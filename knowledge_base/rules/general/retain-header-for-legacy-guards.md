---
name: skia/retain-header-for-legacy-guards
description: When removing header includes from files containing SK_SUPPORT_LEGACY_ guards, retain declarations or conditionally include the header until legacy code is removed
rubric: correctness
severity: blocking
---

**Rule.** Do not remove a header include from a source or header file if the
file still contains conditionally compiled blocks (such as '#if
defined(SK_SUPPORT_LEGACY_...)') implementing or referencing types declared in
that header. Either retain the include (nested inside the conditional
preprocessor block if appropriate) or verify that downstream embedders like
Chromium have migrated before removing the header.

**Why.** In standalone Skia builds, legacy deprecation macros are often unset or
defined in the legacy header itself, so removing the include silently compiles
without error in local CI by skipping the guarded block. Downstream embedders
like Chromium define these legacy macros at the build level, causing compilation
failures when legacy blocks compile without required declarations.

## Examples

Bad:

```cpp
// Removed #include "include/effects/SkGradientShader.h"
#include "include/effects/SkGradient.h"

// Fails in Chromium where SK_SUPPORT_LEGACY_UNSPANNED_GRADIENTS is defined in GN
#if defined(SK_SUPPORT_LEGACY_UNSPANNED_GRADIENTS)
sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2], ...) { ... }
#endif
```

Good:

```cpp
#include "include/effects/SkGradient.h"

#if defined(SK_SUPPORT_LEGACY_UNSPANNED_GRADIENTS)
#include "include/effects/SkGradientShader.h"

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2], ...) { ... }
#endif
```
