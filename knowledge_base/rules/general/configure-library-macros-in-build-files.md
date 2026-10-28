---
name: skia/configure-library-macros-in-build-files
description: 'When configuring feature or library macros, set them in build files or check existing build flags directly rather than hardcoding local #define directives in source files'
rubric: readability
severity: should_fix
---

**Rule.** Do not hardcode #define directives for build-level, platform, or
third-party library configuration macros, nor define local macro aliases
wrapping existing build configuration flags, in C++ source or header files.
Check existing build configuration flags directly, or configure feature and
library defines in build files (defines in BUILD.gn or local_defines in Bazel)
if independent configurability is required.

**Why.** Hardcoding configuration macros in source files conflicts with
command-line -D compiler flags and can cause -Wmacro-redefined warnings, while
local macro aliases add unnecessary indirection and preprocessor pollution.
Configuring defines in build files ensures clean target scoping, and checking
existing build flags directly avoids misleading pseudo-configurability.

## Examples

Bad:

```cpp
// Bad: hardcoding library configuration macros in source files
#define U_LIB_SUFFIX_C_NAME _skia
#define U_HAVE_LIB_SUFFIX 1
#define U_DISABLE_RENAMING 0

// Bad: defining local macro aliases for existing build configuration flags
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
#define SK_AVIF_USE_CRABBYAVIF 1
#else
#define SK_AVIF_USE_CRABBYAVIF 0
#endif
```

Good:

```cpp
// Good: configure library defines in BUILD.gn or Bazel
skia_component("skunicode_libgrapheme") {
  defines = [ "U_HAVE_LIB_SUFFIX=1", "U_DISABLE_RENAMING=0" ]
  sources = [ "src/SkBidiFactory_icu_subset.cpp" ]
}

// Good: check existing build configuration flags directly
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
namespace crabbyavif {
struct avifDecoder;
}
#endif
```
