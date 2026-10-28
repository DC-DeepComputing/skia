---
name: skia/modules/avoid-test-font-mgr-in-standalone-tools
description: In standalone CLI tools and production binaries, instantiate platform SkFontMgr directly instead of calling ToolUtils::TestFontMgr() or linking tool_utils
rubric: safety
severity: should_fix
---

**Rule.** In standalone CLI tools, production binaries, and non-test modules,
instantiate platform SkFontMgr instances directly (or accept an injected font
manager) rather than calling ToolUtils::TestFontMgr() or depending on
//tools:tool_utils. Reserve ToolUtils::TestFontMgr() strictly for tests, GMs,
benchmarks, and test-only harnesses.

**Why.** ToolUtils::TestFontMgr() and //tools:tool_utils pull in bundled test
fonts, command-line flag parsers, and test-framework symbols. Linking test
utilities into standalone executables or non-test modules significantly bloats
binary size and introduces unwanted dependencies on testing infrastructure.

## Examples

Bad:

```cpp
#include "tools/fonts/FontToolUtils.h"
// BUILD deps: "//tools:tool_utils"

auto anim = skottie::Animation::Builder()
        .setFontManager(ToolUtils::TestFontMgr())
        .build();
```

Good:

```cpp
#if defined(SK_BUILD_FOR_MAC) && defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_CoreText(nullptr);
#elif defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_Android(nullptr, SkFontScanner_Make_FreeType());
#elif defined(SK_BUILD_FOR_UNIX) && defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
#else
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_Custom_Empty();
#endif

auto anim = skottie::Animation::Builder()
        .setFontManager(std::move(fontMgr))
        .build();
```
