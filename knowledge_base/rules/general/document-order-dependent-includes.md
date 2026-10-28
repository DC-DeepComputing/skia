---
name: skia/document-order-dependent-includes
description: 'When #include directives must follow a specific non-alphabetical sequence, document the required order and its purpose in an explanatory comment'
rubric: readability
severity: nit
---

**Rule.** When #include directives must appear in a specific non-alphabetical or
order-dependent sequence (such as sandwiching an implementation header between
compiler target-attribute headers like SkOpts_SetTarget.h and
SkOpts_RestoreTarget.h, or macro configuration wrappers), document the required
order and its purpose in an explanatory comment. Detail each step of the
inclusion sequence so maintainers and automated formatting tools do not reorder
or consolidate the includes.

**Why.** Skia coding style expects include directives to be independent and
sorted alphabetically. When files rely on order-dependent includes to configure
compiler pragmas or macro state, the non-standard order appears arbitrary;
documenting the sequence prevents maintainers or automated formatters from
inadvertently alphabetizing or reordering the headers and breaking the build.

## Examples

Bad:

```cpp
#define SK_OPTS_TARGET SK_OPTS_TARGET_HSW
#include "src/opts/SkOpts_SetTarget.h"

#include "src/core/SkBitmapProcState.h"
#include "src/opts/SkBitmapProcState_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"
```

Good:

```cpp
// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.h file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_HSW
#include "src/opts/SkOpts_SetTarget.h"

#include "src/core/SkBitmapProcState.h"
#include "src/opts/SkBitmapProcState_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"
```
