---
name: skia/include-header-declaring-exported-apis
description: 'When defining public or module API functions in C++ source files, #include the declaring header and omit SK_API on function definitions'
rubric: readability
severity: should_fix
---

**Rule.** Source files (.cpp) that implement public or module API functions must
#include the header that declares them with symbol export macros (such as SK_API
or <MODULE>_API), rather than relying solely on base class or umbrella headers.
Additionally, do not annotate function definitions in .cpp source files with
SK_API; place symbol export macros exclusively on declarations in public
headers.

**Why.** In shared library and component builds, symbol export macros expand to
platform-specific visibility attributes (__declspec(dllexport) or default
visibility). If a translation unit defines an exported function without
including its declaring header, the compiled object omits export attributes,
causing unresolved symbol or missing export link errors when consumers link
against the shared library. Furthermore, placing export macros directly on
function definitions in source files bypasses header validation and violates
symbol export conventions.

## Examples

Bad:

```cpp
// modules/skunicode/src/SkUnicode_libgrapheme.cpp
#include "modules/skunicode/include/SkUnicode.h"
// Missing #include "modules/skunicode/include/SkUnicode_libgrapheme.h"
namespace SkUnicodes::Libgrapheme {
sk_sp<SkUnicode> Make() { return sk_make_sp<SkUnicode_libgrapheme>(); }
}

// src/ports/SkFontMgr_fontations_empty.cpp (without including SkFontMgr_Fontations.h)
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fontations_Empty() {
    return sk_make_sp<SkFontMgr_Fontations_Empty>();
}
```

Good:

```cpp
// modules/skunicode/src/SkUnicode_libgrapheme.cpp
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
namespace SkUnicodes::Libgrapheme {
sk_sp<SkUnicode> Make() { return sk_make_sp<SkUnicode_libgrapheme>(); }
}

// src/ports/SkFontMgr_fontations_empty.cpp
#include "include/ports/SkFontMgr_Fontations.h"
sk_sp<SkFontMgr> SkFontMgr_New_Fontations_Empty() {
    return sk_make_sp<SkFontMgr_Fontations_Empty>();
}
```
