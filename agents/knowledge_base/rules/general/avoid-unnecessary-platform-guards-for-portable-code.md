---
name: skia/avoid-unnecessary-platform-guards-for-portable-code
description: When writing cross-platform or platform-specific code, omit unnecessary preprocessor guards for portable operations, supported OS targets, or runtime-failing APIs
rubric: design
severity: should_fix
---

**Rule.** Do not guard portable library calls, configuration settings, or bug
mitigations behind platform-specific preprocessor conditionals (such as `#if
defined(SK_BUILD_FOR_WIN)`) when logic is valid across all platforms; execute
them unconditionally. Additionally, when implementing architecture-specific
features using CPU intrinsics or inline assembly (e.g., under `SK_CPU_X86` or
`SK_CPU_ARM64`), do not preemptively stub out supported operating systems (such
as `SK_BUILD_FOR_MAC`) with dummy fallbacks unless an OS-specific limitation
prevents their use. Furthermore, avoid compile-time preprocessor guards (such as
`#ifdef WINUWP`) to stub out APIs that fail gracefully at runtime (e.g., via
`CoCreateInstance`), letting runtime discovery and caller error handling manage
unsupported variants unless symbols are missing.

**Why.** Unnecessary platform preprocessor conditionals introduce artificial
platform divergence, dead fallback code, and maintenance overhead. Preemptively
stubbing out supported operating systems prevents capable platforms from using
architecture optimizations, while guarding runtime-discoverable APIs with
`#ifdef` duplicates error-handling logic that runtime fallbacks already handle
naturally.

## Examples

Bad:

```cpp
// Guarding portable logic behind platform preprocessor checks:
#if defined(SK_BUILD_FOR_WIN)
    XML_SetHashSalt(ctx.fXMLParser, seed ? seed : 1);
#endif

// Preemptively stubbing out supported OS before architecture check:
#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    return 0ul;
#elif defined(SK_CPU_X86)
    return __builtin_ia32_rdtscp(&cpuInfo);
#endif

// Preprocessor stub for APIs that fail gracefully at runtime:
#ifdef WINUWP
    *image = nullptr;
    return S_OK;
#else
    HRM(CoCreateInstance(CLSID_XpsOMThumbnailGenerator, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&thumbnailGenerator)),
        "Could not create thumbnail generator.");
#endif
```

Good:

```cpp
// Execute portable logic unconditionally across platforms:
    XML_SetHashSalt(ctx.fXMLParser, seed ? seed : 1);

// Retain architecture intrinsics on supported OS targets:
#if defined(SK_BUILD_FOR_WIN)
    return 0ul;
#elif defined(SK_CPU_X86)
    return __builtin_ia32_rdtscp(&cpuInfo);
#endif

// Rely on runtime activation failure and caller error handling:
    HRM(CoCreateInstance(CLSID_XpsOMThumbnailGenerator, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&thumbnailGenerator)),
        "Could not create thumbnail generator.");
```
