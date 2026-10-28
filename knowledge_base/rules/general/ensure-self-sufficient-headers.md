---
name: skia/ensure-self-sufficient-headers
description: When creating or modifying C++ headers, ensure standalone compilation by directly including defining headers for value types, standard headers, and macros, while forward-declaring pointer/reference types
rubric: readability
severity: blocking
---

**Rule.** Ensure all C++ header files are self-sufficient and compile cleanly as
standalone translation units. Explicitly include defining headers for types
required by value or definition (such as class data members), standard library
headers (such as <cstdint> for fixed-width integers, or <vector>), and macros or
utilities (such as include/private/base/SkMacros.h for SK_MACRO_STRINGIFY),
rather than relying on transitive includes. When writing parallel implementation
headers (such as SIMD and scalar tiers), ensure both directly include their
required types. For project types referenced only by pointer or reference,
prefer alphabetically ordered forward declarations. In Bazel build files, ensure
headers are registered in generate_cpp_files_for_headers.

**Why.** Downstream embedders and environments like Google3 compile header files
independently. Relying on transitive includes or enclosing translation units
breaks standalone compilation across different platforms when headers are
refactored. Directly including types used by value, standard library types
(which cannot be forward-declared), and required macro definitions, while using
alphabetical forward declarations for pointer and reference types, keeps headers
self-contained and minimizes rebuild cascades.

## Examples

Bad:

```cpp
// 1. Missing <cstdint> when using fixed-width integers in header declarations:
template <uint16_t kTileWidth, uint16_t kTileHeight>
class StripProcessorScalar {
    uint8_t fMask; // Missing #include <cstdint>
};

// 2. Relying on transitive includes for types held by value:
#include "include/codec/SkCodec.h"
class ColorProfile {
    skcms_ICCProfile fProfile; // Missing direct #include "modules/skcms/skcms.h"
};

// 3. Standard library types cannot be forward-declared; relying on transitive includes:
#include "webgpu/webgpu_cpp.h"
void AddFeatures(const wgpu::Adapter& adapter, std::vector<wgpu::FeatureName>& features); // Missing #include <vector>

// 4. Missing forward declaration relies on transitive/preceding includes:
struct SkRect;
namespace SkPathRawShapes {
struct RRect : public SkPathRaw {
    RRect(const SkRRect&, SkPathDirection dir, unsigned index); // SkRRect undeclared
};
}

// 5. Using macros without directly including defining header:
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#define GR_FILE_AND_LINE_STR __FILE__ "(" SK_MACRO_STRINGIFY(__LINE__) ") : " // Missing SkMacros.h
```

Good:

```cpp
// 1. Directly include <cstdint> when using fixed-width integers:
#include <cstdint>
template <uint16_t kTileWidth, uint16_t kTileHeight>
class StripProcessorScalar {
    uint8_t fMask;
};

// 2. Directly include defining header for types held by value:
#include "include/codec/SkCodec.h"
#include "modules/skcms/skcms.h"
class ColorProfile {
    skcms_ICCProfile fProfile;
};

// 3. Directly include standard library headers when standard types are used:
#include "webgpu/webgpu_cpp.h"
#include <vector>
void AddFeatures(const wgpu::Adapter& adapter, std::vector<wgpu::FeatureName>& features);

// 4. Alphabetically ordered forward declarations keep header self-contained:
class SkRRect;
struct SkRect;
namespace SkPathRawShapes {
struct RRect : public SkPathRaw {
    RRect(const SkRRect&, SkPathDirection dir, unsigned index);
};
}

// 5. Directly include defining header for macros:
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/base/SkMacros.h"
#define GR_FILE_AND_LINE_STR __FILE__ "(" SK_MACRO_STRINGIFY(__LINE__) ") : "
```
