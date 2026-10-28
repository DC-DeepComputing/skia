---
name: skia/api/prefix-non-file-scope-macros
description: When defining preprocessor macros with greater than file scope in headers, prefix macro names with SK_ or GR_ rather than generic or unprefixed identifiers
rubric: readability
severity: should_fix
---

**Rule.** Prefix preprocessor macros that have greater than file scope (such as
macros defined in header files) with SK_, GR_ (in legacy Ganesh), or an explicit
subsystem prefix (e.g., SKGPU_ or SK_<SUBSYSTEM>_). Do not introduce unprefixed,
generic, or lowercase macro names in header files.

**Why.** Preprocessor macros lack C++ namespace scoping and leak into every
translation unit that includes the header. Unprefixed or generic macro
identifiers (such as NO_CODE or START_*) pollute the global preprocessor
namespace and risk silent symbol collisions across translation units.

## Examples

Bad:

```cpp
// src/utils/SkBlitterTrace.h
#define NO_CODE do {} while (0)
#define START_BLITTER_TRACE(blitter) ...
#define RESTORE_BLITTER_TRACE(blitter) NO_CODE
```

Good:

```cpp
// src/utils/SkBlitterTrace.h
#define SK_BLITTER_TRACE_NO_CODE do {} while (0)
#define SK_BLITTER_TRACE_START(blitter) ...
#define SK_BLITTER_TRACE_RESTORE(blitter) SK_BLITTER_TRACE_NO_CODE
```
