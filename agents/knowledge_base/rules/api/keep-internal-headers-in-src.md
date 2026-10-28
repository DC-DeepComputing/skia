---
name: skia/api/keep-internal-headers-in-src
description: In public headers under include/ or modules/, do not include private headers, declare internal types, or expose internal or skia_private types in signatures or aliases
rubric: design
severity: should_fix
---

**Rule.** Place internal implementation headers, types, and enums in src/ (or in
include/private/ when required by public headers); do not declare internal-only
types, enums, or helper definitions in public headers. Conversely, types or
factories required to call or configure a public API (such as
SkFontScanner_Make_FreeType) must be exposed in a public header under include/
rather than trapped in src/. Public headers must not include private headers or
expose internal types (from src/ or namespace skia_private, such as STArray or
TArray) in public aliases or signatures; use public Skia types (such as SkV2,
SkSpan) or standard types (such as std::vector) instead.

**Why.** Public include directories define Skia's external API surface, and
external clients do not have src/ in their header include paths. Confining
internal headers, types, and enums to src/ prevents leaking implementation
details into client code and namespace pollution, while exposing required API
types and factories in include/ ensures downstream embedders can configure and
consume public APIs cleanly. Exposing skia_private types leaks internal
container symbols and forces downstream clients to depend on private
implementation details.

## Examples

Bad:

```cpp
// modules/skparagraph/include/ParagraphCache.h (declaring internal enum in public header)
enum InternalState { kUnknown = 0, kIndexed = 1, kShaped = 2 };

// include/core/SkPixelStorage.h (internal class exposed in public API header)
class SkPixelStorage : public SkRefCnt { ... };

// include/private/gpu/graphite/DebugUtilsGraphite.h (internal header not needed by public headers)
#define SK_DUMP_TASKS_CODE(...) __VA_ARGS__

// modules/skottie/include/Skottie.h (including private implementation header in public header)
#include "include/private/base/SkSpan_impl.h"

// modules/skplaintexteditor/app/editor_application.cpp (requiring internal src/ header to use public API)
#include "include/ports/SkFontMgr_fontconfig.h"
#include "src/ports/SkTypeface_FreeType.h"
auto fontMgr = SkFontMgr_New_FontConfig(nullptr, std::make_unique<SkFontScanner_FreeType>());

// modules/skottie/include/SlotManager.h (exposing module-internal src/ types in public signatures)
void setScalarSlot(SlotID, ScalarValue);

// modules/svg/include/SkSVGTypes.h (exposing skia_private container in public type alias)
class SK_API SkSVGColor {
public:
    using Vars = skia_private::STArray<1, SkString>;
};
```

Good:

```cpp
// modules/skparagraph/src/ParagraphImpl.h (internal enum placed in internal header)
enum InternalState { kUnknown = 0, kIndexed = 1, kShaped = 2 };

// include/private/SkPixelStorage.h (placed in include/private/ because public SkPixelRef.h needs it)
class SkPixelStorage : public SkRefCnt { ... };

// src/gpu/graphite/DebugUtils.h (internal header confined to src/)
#define SK_DUMP_TASKS_CODE(...) __VA_ARGS__

// modules/skottie/include/Skottie.h (including public API header)
#include "include/core/SkSpan.h"

// include/ports/SkFontScanner_FreeType.h exposes factory; client application includes public header
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
auto fontMgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());

// modules/skottie/include/SlotManager.h (using public Skia types and primitives in signatures)
bool setScalarSlot(const SlotID&, float);

// modules/svg/include/SkSVGTypes.h (using standard or public vocabulary types in public interface)
class SK_API SkSVGColor {
public:
    using Vars = std::vector<SkString>;
};
```
