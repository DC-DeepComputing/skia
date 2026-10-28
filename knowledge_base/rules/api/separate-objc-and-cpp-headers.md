---
name: skia/api/separate-objc-and-cpp-headers
description: 'When exposing Metal or Objective-C GPU APIs in public headers, separate Objective-C types and C++ utilities into distinct headers instead of #if __OBJC__'
rubric: design
severity: should_fix
---

**Rule.** Separate Objective-C-only type definitions (such as types depending on
<Metal/Metal.h>) and C++-compatible utility declarations into distinct header
files rather than multiplexing them in a single header with #if __OBJC__.
Provide a C++-compatible header (e.g., using CFTypeRef and forward declarations)
so pure C++ translation units can include the APIs cleanly without requiring
Objective-C compilation.

**Why.** Headers importing <Metal/Metal.h> or defining Objective-C types fail to
compile in pure C++ translation units. Using #if __OBJC__ within a single header
to conditionally define or forward-declare types complicates include graphs and
prevents C++ translation units from cleanly consuming utility functions.

## Examples

Bad:

```cpp
// MtlGraphiteTypes.h
#if __OBJC__
#import <Metal/Metal.h>
struct SK_API MtlTextureInfo { ... };
#else
struct SK_API MtlTextureInfo;
#endif

namespace skgpu::graphite::TextureInfos {
SK_API TextureInfo MakeMetal(CFTypeRef mtlTexture);
}
```

Good:

```cpp
// MtlGraphiteTypes_cpp.h (pure C++ compatible)
#import <CoreFoundation/CoreFoundation.h>

namespace skgpu::graphite {
struct SK_API MtlTextureInfo;
namespace TextureInfos {
SK_API TextureInfo MakeMetal(CFTypeRef mtlTexture);
}
}  // namespace skgpu::graphite

// MtlGraphiteTypes.h (Objective-C++ only)
#if __OBJC__
#import <Metal/Metal.h>
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"

namespace skgpu::graphite {
struct SK_API MtlTextureInfo { ... };
}  // namespace skgpu::graphite
#endif
```
