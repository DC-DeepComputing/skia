---
name: skia/prefix-non-file-scoped-macros-with-sk
description: When defining preprocessor macros, use standard SK_ or subsystem-specific prefixes (like SKRP_ or GR_GL_), and maintain consistent hierarchical component-action names
rubric: readability
severity: should_fix
---

**Rule.** Prefix preprocessor macros that have greater than file scope with SK_
(or GR_ in legacy Ganesh code) or an explicit, accurate subsystem prefix (e.g.,
SKRP_ for Raster Pipeline, SKSL_ for SkSL, or GR_GL_ for OpenGL-specific Ganesh
macros). Match the prefix specifically to the target subsystem being configured
or instrumented rather than using a broader or adjacent subsystem identifier.
When defining related macro families within a feature, follow a consistent
<PREFIX>_<SUBSYSTEM>_<ACTION> hierarchy (such as SK_BLITTER_TRACE_STEP) rather
than leading action verbs. For macros selecting between mutually exclusive
backend implementations or variants, use a consistent subsystem prefix and
variant descriptor (e.g., SK_<SUBSYSTEM>_IS_<VARIANT>) rather than generic
boolean toggles.

**Why.** Preprocessor macros lack C++ namespace scoping and leak across
translation units. Using standard project or subsystem prefixes avoids symbol
collisions with system headers and third-party code. Matching the prefix to the
exact subsystem prevents confusion with unrelated feature toggles, while
hierarchical component-action naming and explicit variant descriptors make macro
suites cohesive and easy to discover.

## Examples

Bad:

```cpp
// Bad: Leading action verbs, generic toggles, and mismatched subsystem prefixes
#define PRINT_BLITTER_TRACE(trace) ...
#define STEP_BLITTER_TRACE(name, trace, scanlines, pixels) ...

#define SKVM_BLITTER_TRACE_ON
#include "src/utils/SkBlitterTrace.h"

// In Raster Pipeline code: using general SkSL prefix instead of SKRP_
#define SKSL_ENABLE_LIVE_TRACE 0
#if SKSL_ENABLE_LIVE_TRACE
    tempDebugTrace = SkSL::DebugTraceImpl::Make();
#endif
```

Good:

```cpp
// Good: Prefixed with SK_, structured hierarchy, explicit variant, and accurate subsystem acronym
#define SK_BLITTER_TRACE_PRINT(trace) ...
#define SK_BLITTER_TRACE_STEP(name, trace, scanlines, pixels) ...

#define SK_BLITTER_TRACE_IS_SKVM
#include "src/utils/SkBlitterTrace.h"

// In Raster Pipeline code: accurate subsystem prefix
#define SKRP_ENABLE_LIVE_TRACE 0
#if SKRP_ENABLE_LIVE_TRACE
    tempDebugTrace = SkSL::DebugTraceImpl::Make();
#endif
```
