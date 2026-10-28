---
name: skia/ports/provide-fallback-macro-definitions-for-external-libraries
description: 'When using newer external library constants in platform ports, provide fallback #ifndef macro definitions with introduction comments instead of hardcoding literals with TODOs'
rubric: readability
severity: should_fix
---

**Rule.** When using constants, keys, or flags introduced in newer versions of
external platform libraries (such as FontConfig or FreeType) in src/ports/,
provide a fallback definition guarded by #ifndef <MACRO> (annotated with a
comment citing the library version that introduced it). Do not hardcode magic
numbers, raw string literals, or temporary TODO comments waiting for downstream
sysroot updates.

**Why.** External platform dependencies like FontConfig or FreeType vary across
client sysroots and build environments. Defining fallback macros annotated with
their upstream release version lets port code immediately use standard,
descriptive API identifiers without risking build breaks on older toolchains or
accumulating forgotten TODO debt.

## Examples

Bad:

```cpp
// TODO(https://crbug.com/439788709): Move the key name to FC_FONT_WRAPPER
// once the build issue is fixed on the Chrome side.
FcPatternAddString(pattern, "fontwrapper", reinterpret_cast<const FcChar8*>("SFNT"));
```

Good:

```cpp
// Available since FontConfig 2.15.
#ifndef FC_FONT_WRAPPER
#define FC_FONT_WRAPPER         "fontwrapper"
#endif
...
FcPatternAddString(pattern, FC_FONT_WRAPPER, reinterpret_cast<const FcChar8*>("SFNT"));
```
