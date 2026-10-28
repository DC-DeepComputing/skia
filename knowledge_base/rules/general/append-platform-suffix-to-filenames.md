---
name: skia/append-platform-suffix-to-filenames
description: When naming platform-specific source or header files specializing a base component, append the platform name as a terminal suffix rather than infixing it
rubric: readability
severity: nit
---

**Rule.** When naming platform-specific source or header files that extend or
specialize a base component or factory, append the platform name as a terminal
suffix (e.g., <Base>_<Platform>.cpp) rather than inserting it into the middle of
the filename. Keep the base component name intact at the beginning of the
filename so platform variants group together alphabetically.

**Why.** Appending the platform name as a terminal suffix keeps
platform-specific files consistently aligned with the base component name in
directory listings, build file source lists, and search results, preventing
fragmentation across alphabetically sorted lists.

## Examples

Bad:

```cpp
// When extending SkImage_GaneshFactories for Android:
src/gpu/ganesh/image/SkImage_AndroidGaneshFactories.cpp
```

Good:

```cpp
// When extending SkImage_GaneshFactories for Android:
src/gpu/ganesh/image/SkImage_GaneshFactories_Android.cpp
```
