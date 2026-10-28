---
name: skia/omit-redundant-android-api-checks-in-framework
description: When guarding code for Android framework builds with SK_BUILD_FOR_ANDROID_FRAMEWORK, omit redundant __ANDROID_API__ version checks as the platform always compiles against current APIs
rubric: readability
severity: should_fix
---

**Rule.** Do not combine `__ANDROID_API__` preprocessor version checks with `#if
defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)`. When guarding code specifically
compiled for the Android OS framework tree, omit platform API level checks
because framework builds always target the current platform API baseline.

**Why.** Skia compiles inside the Android OS framework tree
(`SK_BUILD_FOR_ANDROID_FRAMEWORK`) against the latest platform API headers.
Checking legacy API levels (such as `__ANDROID_API__ >= 28`) inside
framework-only code is always true and well outside any cherry-pick range,
creating unnecessary preprocessor clutter.

## Examples

Bad:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && __ANDROID_API__ >= 28
    case AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM:
        return kBGRA_8888_SkColorType;
#endif
```

Good:

```cpp
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    case AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM:
        return kBGRA_8888_SkColorType;
#endif
```
