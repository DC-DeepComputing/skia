---
name: skia/modules/enforce-canvaskit-prerequisites-with-error
description: 'When authoring modular CanvasKit C++ binding files that require optional components or build flags, enforce prerequisites at file top with #if and #error'
rubric: correctness
severity: should_fix
---

**Rule.** In modular CanvasKit C++ source files or bindings that depend on
optional Skia components, modules, or build flags (such as SkUnicode features or
codec decoders), enforce the prerequisite at the top of the file using
preprocessor `#if !defined(...)` (or `#if defined(...) ... #else`) with
`#error`. Provide a clear diagnostic message identifying the missing component
so misconfigured builds fail immediately.

**Why.** CanvasKit supports fine-grained modular builds where optional
subsystems can be excluded. If a modular binding file is included in a build
target without its underlying prerequisite feature enabled, compilation or
linking fails with obscure downstream errors (such as missing declarations or
unresolved symbols) rather than an immediate, actionable diagnostic indicating
which build flag is missing.

## Examples

Bad:

```cpp
// modules/canvaskit/bidi_bindings.cpp
#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_BIDI_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_bidi.h"
#endif

// Subsequent code unconditionally references SkUnicodes::Bidi, failing cryptically
// with missing symbol or undeclared identifier errors if the flag is unset.
```

Good:

```cpp
// modules/canvaskit/bidi_bindings.cpp
#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_BIDI_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_bidi.h"
#else
#error "SkUnicode bidi component is required but missing"
#endif
```
