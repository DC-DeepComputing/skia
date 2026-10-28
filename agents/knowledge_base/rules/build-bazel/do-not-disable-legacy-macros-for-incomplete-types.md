---
name: skia/build-bazel/do-not-disable-legacy-macros-for-incomplete-types
description: When resolving incomplete-type compiler errors under SK_DISABLE_LEGACY_* macros, include required headers directly in source files instead of disabling deprecation macros in build targets
rubric: design
severity: should_fix
---

**Rule.** Do not disable, unset, or conditionally omit legacy-deprecation macros
(such as SK_DISABLE_LEGACY_*) in GN or Bazel build targets to work around
incomplete-type compiler errors. Instead, retain the deprecation define and
directly #include the header defining the required type in the source (.cpp)
file that uses it.

**Why.** Legacy-deprecation macros enforce modularity and loose coupling by
replacing heavy #include directives in public headers with forward declarations.
Omitting or disabling these macros in build targets pulls in obsolete legacy
declarations and masks missing direct dependencies, violating IWYU
(include-what-you-use) principles and undermining API migration goals.

## Examples

Bad:

```cpp
# In modules/skshaper/BUILD.bazel:
# Disabling deprecation macro on Windows to bypass incomplete SkFontMgr error
defines = select({
    "@platforms//os:windows": [],
    "//conditions:default": [
        "SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS",
    ],
})
```

Good:

```cpp
# In modules/skshaper/BUILD.bazel:
defines = ["SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS"],

// In modules/skshaper/src/SkShaper_factory.cpp:
// Directly include defining header for SkFontMgr rather than disabling deprecation
#include "include/core/SkFontMgr.h"
#include "modules/skshaper/include/SkShaper_factory.h"
```
