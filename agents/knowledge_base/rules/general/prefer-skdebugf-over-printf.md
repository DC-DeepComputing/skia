---
name: skia/prefer-skdebugf-over-printf
description: When logging debug messages or diagnostics, use Skia's platform-aware SkDebugf instead of standard library printf or std::printf
rubric: readability
severity: should_fix
---

**Rule.** Do not use standard library `printf` or `std::printf` for debug
logging or diagnostics; use Skia's platform-aware logging function `SkDebugf`
(or `SkDEBUGF`) instead.

**Why.** Skia runs on diverse target platforms (such as Android, iOS, Windows,
and WebAssembly) where stdout output from standard `printf` is often dropped,
redirected, or inaccessible. `SkDebugf` routes output through Skia's
platform-specific logging backends (such as Android logcat), ensuring
diagnostics reach the appropriate system logging facility.

## Examples

Bad:

```cpp
#define RETURN_ON_FALSE(x) \
    do { \
        if (!(x)) { \
            printf("Failed at %d\n", __LINE__); \
            return false; \
        } \
    } while (0)
```

Good:

```cpp
#define RETURN_ON_FALSE(x) \
    do { \
        if (!(x)) { \
            SkDebugf("Parsing failed %s at %d\n", #x, __LINE__); \
            return false; \
        } \
    } while (0)
```
