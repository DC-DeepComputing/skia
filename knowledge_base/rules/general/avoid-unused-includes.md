---
name: skia/avoid-unused-includes
description: 'When managing #include directives, forward declarations, and using-declarations, avoid unused dependencies, prefer forward declarations when complete layout is unneeded, and remove obsolete declarations'
rubric: readability
severity: blocking
---

**Rule.** Do not include unused or duplicate headers, or add unreferenced using
namespace directives (e.g., using namespace skia_private;), and remove obsolete
forward declarations and unused type aliases (using / typedef) when types are no
longer referenced. In header files, minimize includes by preferring
alphabetically ordered forward declarations whenever a complete type definition
is not required—including for pointer, reference, smart pointer (sk_sp<T>,
std::unique_ptr<T>), and function parameter or return types passed by value in
declarations. Only include defining headers when complete type layout is needed
(such as for data members, base classes, or inline function bodies). When types
are only referenced via sk_sp<T>, include "include/core/SkRefCnt.h" instead of
the defining header for T (ensuring destructors are out-of-line). In source
files, avoid including full header definitions when passing nullptr to functions
with forward-declared parameter types.

**Why.** Unused or duplicate includes, unreferenced using-namespace directives,
obsolete forward declarations, and orphaned type aliases clutter interfaces,
confuse readers about dependencies, and bloat translation units, increasing
compilation and incremental rebuild times. Preferring forward declarations over
includes for non-inlined function signatures, references, pointers, and smart
pointers avoids transitive include bloat and cascading rebuilds across the
codebase when headers change. For ref-counted types, including lightweight
SkRefCnt.h with forward declarations avoids pulling in heavy class definitions.

## Examples

Bad:

```cpp
// Header: unnecessary includes when forward declaration suffices
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "src/codec/SkPngCompositeChunkReader.h"
#include <functional>

class SkCaptureCanvas; // unused in header
using AddToKeyFn = std::function<void()>; // obsolete alias after refactoring

class SkFontScanner {
public:
    virtual ~SkFontScanner();
    virtual sk_sp<SkTypeface> makeTypeface(const SkFontArguments& args) const = 0;
    static bool ProcessPath(SkPath path); // by-value param in decl does not need SkPath.h
};

// Source: unused or duplicate headers, unreferenced using-namespace, or include when passing nullptr
#include <cstdint>
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DrawContext.h"
#include <cstdint> // duplicate include
using namespace skia_private; // no skia_private symbols used

GenerateMipmaps(recorder, nullptr, view.refProxy(), info);
```

Good:

```cpp
// Header: forward declarations in alphabetical order; SkRefCnt.h for sk_sp<T>
#include "include/core/SkRefCnt.h"

class SkCanvas;
class SkPath;
class SkPngCompositeChunkReader;
class SkTypeface;
struct SkFontArguments;

class SkFontScanner {
public:
    virtual ~SkFontScanner();
    virtual sk_sp<SkTypeface> makeTypeface(const SkFontArguments& args) const = 0;
    static bool ProcessPath(SkPath path);
};

// Source: DrawContext.h omitted when passing nullptr; omit unused or duplicate headers
#include <cstdint>

GenerateMipmaps(recorder, nullptr, view.refProxy(), info);
```
