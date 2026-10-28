---
name: skia/ports/preserve-fontconfig-fclocker-thread-safety
description: When modifying FontConfig port code in src/ports, retain FCLocker synchronization and FCLocker::AssertHeld() assertions to support older thread-unsafe FontConfig releases
rubric: safety
severity: should_fix
---

**Rule.** In FontConfig port implementations (such as SkFontMgr_fontconfig and
SkFontConfigInterface_direct), retain FCLocker locking and
FCLocker::AssertHeld() assertions around all operations accessing FontConfig
data structures or APIs. Do not remove locking or assertions under the
assumption that FontConfig is inherently thread-safe; older FontConfig releases
(< 2.13.93) remain supported and require external synchronization.

**Why.** FontConfig was not thread-safe prior to version 2.13.93, where internal
mutex locking was introduced. Skia continues to support Linux distributions
running older FontConfig releases, so omitting FCLocker synchronization or
FCLocker::AssertHeld() checks causes data races, memory corruption, and
multithreaded crashes on supported platforms.

## Examples

Bad:

```cpp
// Accessing or modifying FontConfig patterns without asserting or holding FCLocker:
static void remove_weak(FcPattern* pattern, const char object[]) {
    SkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
    FcPattern* stripped = FcPatternFilter(pattern, requestedObjectOnly);
    // ...
}
```

Good:

```cpp
// Asserting FCLocker is held (or acquiring FCLocker lock) before invoking FontConfig APIs:
static void remove_weak(FcPattern* pattern, const char object[]) {
    FCLocker::AssertHeld();

    SkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
    FcPattern* stripped = FcPatternFilter(pattern, requestedObjectOnly);
    // ...
}
```
