---
name: skia/keep-platform-agnostic-utilities-in-common-directories
description: When authoring platform-agnostic utilities or math helpers, place them in common directories rather than platform-specific subdirectories like win/ or mac/
rubric: design
severity: nit
---

**Rule.** Do not place platform-agnostic utilities, math algorithms, or helper
classes in platform-specific subdirectories (such as src/utils/win/,
src/utils/mac/, or platform-specific port folders). Keep portable, reusable code
in common directories (such as src/utils/ or the parent component folder) so it
can be compiled across all platforms without platform coupling.

**Why.** Placing platform-independent code in platform-specific directories
unnecessarily couples portable logic to specific OS build targets, obscures its
general applicability, and hinders reuse across other platforms and common Skia
modules.

## Examples

Bad:

```cpp
// In src/utils/win/SkFloatUtils.h
template <typename RawType, unsigned int ULPs>
class SkFloatingPoint { ... };

// In src/utils/win/SkDWriteGeometrySink.cpp
#include "src/utils/win/SkFloatUtils.h"
```

Good:

```cpp
// In src/utils/SkFloatUtils.h
template <typename RawType, unsigned int ULPs>
class SkFloatingPoint { ... };

// In src/utils/win/SkDWriteGeometrySink.cpp
#include "src/utils/SkFloatUtils.h"
```
