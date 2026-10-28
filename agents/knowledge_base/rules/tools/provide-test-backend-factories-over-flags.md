---
name: skia/tools/provide-test-backend-factories-over-flags
description: 'In test utilities and tests, encapsulate multi-backend fallback in factory helpers like ToolUtils::TestFontScanner() instead of exposing boolean flags or inlining #ifdefs'
rubric: design
severity: should_fix
---

**Rule.** In test and tooling utilities, encapsulate multi-backend fallback and
construction logic inside factory helper functions (such as
ToolUtils::TestFontScanner() or ToolUtils::TestFontMgr()) that return the
configured instance directly, rather than exposing boolean capability flags
(such as TestHasFontations()). In tests, GMs, and benchmarks, use these
centralized factory helpers instead of inlining backend #ifdef checks,
availability flags, and manual fallback instantiation chains.

**Why.** Exposing boolean availability flags from test utility helpers forces
every consuming test or benchmark to duplicate conditional compilation guards,
command-line flag checks, and fallback creation logic. Centralizing instance
creation in test factory helpers encapsulates backend selection in a single
place, ensuring consistent configuration flag handling across test targets while
keeping test files clean and backend-agnostic.

## Examples

Bad:

```cpp
// In test utility (tools/fonts/FontToolUtils.h):
bool TestHasFontations() {
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
    return FLAGS_fontations;
#else
    return false;
#endif
}

// In test callers (tests/ or gm/):
std::unique_ptr<SkFontScanner> scanner = nullptr;
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
if (ToolUtils::TestHasFontations()) {
    scanner = SkFontScanner_Make_Fontations();
}
#endif
#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
if (!scanner) {
    scanner = SkFontScanner_Make_FreeType();
}
#endif
```

Good:

```cpp
// In test utility (tools/fonts/FontToolUtils.cpp):
std::unique_ptr<SkFontScanner> TestFontScanner() {
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
    if (FLAGS_fontations) {
        if (auto result = SkFontScanner_Make_Fontations()) {
            return result;
        }
    }
#endif
#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
    return SkFontScanner_Make_FreeType();
#else
    return nullptr;
#endif
}

// In test callers (tests/ or gm/):
std::unique_ptr<SkFontScanner> scanner = ToolUtils::TestFontScanner();
```
