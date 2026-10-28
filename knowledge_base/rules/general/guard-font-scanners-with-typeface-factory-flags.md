---
name: skia/guard-font-scanners-with-typeface-factory-flags
description: When including platform- or backend-specific font port headers or instantiating scanners, do not include them unconditionally; guard scanners with SK_TYPEFACE_FACTORY_* and managers with SK_FONTMGR_*_AVAILABLE
rubric: correctness
severity: should_fix
---

**Rule.** Do not include platform- or backend-specific port headers (such as
include/ports/SkFontScanner_*.h or include/ports/SkFontMgr_*.h) unconditionally.
Guard font scanner headers and factory calls (such as
SkFontScanner_Make_Fontations() or SkFontScanner_Make_FreeType()) with typeface
factory macros (SK_TYPEFACE_FACTORY_*), never with font manager availability
macros (SK_FONTMGR_*_AVAILABLE). Guard font manager port headers and
instantiations with their corresponding platform and font manager availability
macros (SK_FONTMGR_*_AVAILABLE, SK_BUILD_FOR_*).

**Why.** Port headers expose backend- and platform-specific APIs (such as
FreeType, Fontations, FontConfig, or DirectWrite) that depend on external
libraries and are conditionally compiled. Unconditional inclusion causes build
failures in configurations where dependencies are disabled. Furthermore, font
scanners parse font binaries independently of font managers, so guarding
scanners with font manager flags breaks configurations where a scanner is paired
with a different font manager.

## Examples

Bad:

```cpp
// Bad: Unconditional port include, or guarding font scanners with font manager flags
#include "include/ports/SkFontScanner_Fontations.h"
#include "include/ports/SkFontScanner_FreeType.h"

#if defined(SK_FONTMGR_FONTATIONS_AVAILABLE)
std::unique_ptr<SkFontScanner> scanner = SkFontScanner_Make_Fontations();
#endif
```

Good:

```cpp
// Good: Guard scanner headers and factories with SK_TYPEFACE_FACTORY_*, and managers with SK_FONTMGR_*_AVAILABLE
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "include/ports/SkFontScanner_Fontations.h"
std::unique_ptr<SkFontScanner> scanner = SkFontScanner_Make_Fontations();
#endif

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif
```
