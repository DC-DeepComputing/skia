---
name: skia/prefer-skautolocalesetter-for-locale-overrides
description: When temporarily overriding thread or process locale, use Skia's SkAutoLocaleSetter RAII helper instead of hand-rolling custom locale guards or invoking platform locale APIs
rubric: readability
severity: should_fix
---

**Rule.** When temporarily changing or setting the locale (such as for parsing
numbers, SVGs, shaders, or JSON), use `SkAutoLocaleSetter` rather than
hand-rolling custom RAII locale guards or directly invoking platform-specific
locale APIs (`setlocale`, `uselocale`, `_configthreadlocale`, `newlocale`).
Include `"src/core/SkAutoLocaleSetter.h"` to ensure consistent cross-platform
behavior.

**Why.** `SkAutoLocaleSetter` is Skia's established cross-platform RAII helper
that manages per-thread locale across Windows, POSIX, Apple, and Android (where
locale is a no-op due to NDK limitations). Hand-rolling ad-hoc locale helpers
duplicates boilerplate and risks platform-specific portability bugs or failing
to restore the previous locale upon scope exit.

## Examples

Bad:

```cpp
namespace {
class ScopedThreadLocale {
public:
    explicit ScopedThreadLocale(const char* locale) {
#if defined(SK_BUILD_FOR_WIN)
        _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
        fSavedLocale = std::setlocale(LC_NUMERIC, locale);
#else
        fNewLocale = newlocale(LC_NUMERIC_MASK, locale, LC_GLOBAL_LOCALE);
        fPrevLocale = uselocale(fNewLocale);
#endif
    }
    ~ScopedThreadLocale() { /* restore locale */ }
};
}

ScopedThreadLocale commaLocale("de_DE.UTF-8");
```

Good:

```cpp
#include "src/core/SkAutoLocaleSetter.h"

SkAutoLocaleSetter commaLocale("de_DE.UTF-8");
```
