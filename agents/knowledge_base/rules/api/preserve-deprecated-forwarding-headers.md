---
name: skia/api/preserve-deprecated-forwarding-headers
description: When renaming, moving, or consolidating public headers in include/ or modules/, preserve legacy headers as deprecated forwarding headers to prevent breaking downstream embedders
rubric: design
severity: should_fix
---

**Rule.** When renaming, moving, or consolidating public headers in include/**
or modules/**/include/**, do not delete the original header files. Retain the
legacy header paths as forwarding headers that #include the new header location
and include a deprecation notice comment (e.g., // DEPRECATED: <OldHeader>.h
will be removed in the future, please include <NewHeader>.h), allowing
downstream embedders (Chromium, Android, Flutter) to migrate before removing the
legacy files.

**Why.** External embedders (such as Chromium, Android, and Flutter) depend
directly on Skia's public header paths in their source code and build files.
Abruptly deleting or renaming public headers causes immediate compilation
breakages when Skia is rolled into downstream repositories, whereas transitional
forwarding headers provide a grace period for clients to migrate their include
directives.

## Examples

Bad:

```cpp
// Moving and consolidating DawnTypes.h into DawnGraphiteTypes.h:
// Deleted include/gpu/graphite/dawn/DawnTypes.h
```

Good:

```cpp
// In include/gpu/graphite/dawn/DawnTypes.h:
// DEPRECATED: DawnTypes.h will be removed in the future, please include DawnGraphiteTypes.h
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
```
