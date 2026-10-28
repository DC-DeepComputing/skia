---
name: skia/retain-sktypes-in-headers-with-no-other-includes
description: When writing or refactoring C++ headers that include no other Skia headers, retain an explicit include for SkTypes.h
rubric: readability
severity: should_fix
---

**Rule.** Retain an explicit '#include "include/core/SkTypes.h"' in header files
that include no other Skia headers, even when only forward declarations are used
and static analysis tools like IWYU flag the include as unused.

**Why.** Skia headers rely on SkTypes.h directly or transitively to establish
platform detection macros, compiler features, and build configurations. If a
header with only forward declarations omits SkTypes.h, any conditional
compilation or macro logic added later will lack Skia's configuration
environment, leading to subtle build or macro mismatches.

## Examples

Bad:

```cpp
#ifndef SkClipStackUtils_DEFINED
#define SkClipStackUtils_DEFINED

class SkClipStack;
class SkPath;

void SkClipStack_AsPath(const SkClipStack& cs, SkPath* path);

#endif
```

Good:

```cpp
#ifndef SkClipStackUtils_DEFINED
#define SkClipStackUtils_DEFINED

#include "include/core/SkTypes.h"

class SkClipStack;
class SkPath;

void SkClipStack_AsPath(const SkClipStack& cs, SkPath* path);

#endif
```
