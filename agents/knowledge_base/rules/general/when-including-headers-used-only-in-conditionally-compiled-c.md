---
name: skia/when-including-headers-used-only-in-conditionally-compiled-c
description: When including headers used only in conditionally compiled code, place them inside the matching preprocessor guard or annotate with IWYU pragma keep
rubric: readability
severity: should_fix
---

**Rule.** Place headers needed only for conditionally compiled code inside the
matching #if or #ifdef guard, or annotate them with '// IWYU pragma: keep', to
prevent Include-What-You-Use (IWYU) from reporting them as unused when the
feature is disabled.

**Why.** Headers required solely within conditionally compiled blocks appear
unused to static analysis tools like IWYU in build configurations where the
feature flag is disabled, leading to false-positive unused-include warnings or
automated stripping.

## Examples

Bad:

```cpp
#include <functional>
#include <future>
#include <memory>

#if defined(HAVE_VIDEO_ENCODER)
    #include "experimental/ffmpeg/SkVideoEncoder.h"
#endif
```

Good:

```cpp
#include <functional>
#include <memory>

#if defined(HAVE_VIDEO_ENCODER)
    #include <future>
    #include "experimental/ffmpeg/SkVideoEncoder.h"
#endif
```
