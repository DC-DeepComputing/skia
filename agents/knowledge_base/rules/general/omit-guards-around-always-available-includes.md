---
name: skia/omit-guards-around-always-available-includes
description: When including always-available internal or standard library headers for conditionally compiled code, include them unconditionally rather than wrapping in preprocessor guards
rubric: readability
severity: should_fix
---

**Rule.** Do not conditionally wrap `#include` directives for internal or
standard library headers in `#if defined(...)` or `#ifdef` preprocessor guards
when types are only referenced in conditionally compiled blocks. Include
unconditionally available headers at the top of the file in alphabetical order
alongside other includes, reserving preprocessor guards strictly for platform-
or backend-specific port headers that are unavailable in certain builds.

**Why.** Wrapping always-available internal or standard library headers in `#if
defined(...)` blocks to satisfy strict Include-What-You-Use (IWYU) adds
unnecessary preprocessor boilerplate and breaks alphabetical include
organization. Skia does not enforce automated IWYU for conditional code paths,
so headers that are unconditionally present in the build should be included
unconditionally.

## Examples

Bad:

```cpp
#if defined(SK_DUMP_TASKS)
#include "include/private/base/SkTDArray.h"
#endif

#if defined(SK_DEBUG)
#include <iostream>
#endif
```

Good:

```cpp
#include "include/private/base/SkTDArray.h"

#include <iostream>
```
