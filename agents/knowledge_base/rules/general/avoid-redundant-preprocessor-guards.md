---
name: skia/avoid-redundant-preprocessor-guards
description: 'When code is build-gated, platform-dedicated, or macros have default definitions, omit redundant #if/#ifdef guards and dead #else fallback branches'
rubric: readability
severity: should_fix
---

**Rule.** Do not wrap dedicated, feature-specific source files (.cpp) in
whole-file `#if defined(...)` preprocessor guards; rely on build systems (GN and
Bazel) to conditionally compile the file only when enabled. Additionally, within
files dedicated by build configuration to a single target platform, within
blocks already conditionally compiled under a preprocessor flag, where an
earlier compile-time directive (such as `#error`) guarantees prerequisites, or
where a configuration macro has an unconditional default definition in a central
header (e.g., `SkTypes.h`), do not add redundant downstream `#if`, `#ifdef`, or
`#if defined(...)` guards or dead fallback (`#else`) branches; reference the
macro or implement the guarded behavior directly.

**Why.** Dedicated feature files and target-exclusive components are already
gated by GN and Bazel source lists, so whole-file preprocessor wrappers are
redundant and produce empty translation units when disabled. Similarly,
re-checking an already active preprocessor condition, re-verifying a
prerequisite guaranteed by an earlier `#error`, or adding downstream `#ifdef`
checks and speculative platform fallbacks for macros already unconditionally
defined in central headers creates dead, misleading code in `#else` branches.

## Examples

Bad:

```cpp
// Redundant whole-file guard on a dedicated file already gated by GN/Bazel:
#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
#include "rust/icc/FFI.rs.h"
...
#endif

// Redundant check when macro has default definition in SkTypes.h or earlier #error:
#ifdef SK_GAMMA_CONTRAST
    rec->setContrast(SK_GAMMA_CONTRAST);
#else
    rec->setContrast(0.5f); // Dead fallback
#endif
```

Good:

```cpp
// Rely on GN and Bazel target source lists to conditionally compile dedicated files:
#include "rust/icc/FFI.rs.h"

// Reference macro directly when a central default definition guarantees it is always defined:
rec->setContrast(SK_GAMMA_CONTRAST);
```
