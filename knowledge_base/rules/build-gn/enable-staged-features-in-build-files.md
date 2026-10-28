---
name: skia/build-gn/enable-staged-features-in-build-files
description: When staging behavioral changes, optimizations, or fixes across client rolls, enable flags by default in build files or use opt-out guards
rubric: readability
severity: should_fix
---

**Rule.** When guarding new implementations, optimizations, or behavioral bug
fixes behind preprocessor flags to stage rollouts across downstream clients
(such as Chromium, Flutter, or Google3), enable the flag by default in Skia's
build files (BUILD.gn and Bazel) or invert the preprocessor guard in source code
(#ifndef SK_DISABLE_<FEATURE>, #ifdef SK_IGNORE_<FEATURE>, or #if
!defined(SK_USE_LEGACY_<FEATURE>)) so that Skia CI and unit tests exercise the
new code path immediately. Avoid unconfigured opt-in defines (#ifdef
SK_USE_<FEATURE> or #ifdef SK_<NEW_BEHAVIOR>) that leave new code unexercised by
default in standard Skia builds.

**Why.** Downstream clients maintain independent build files or config headers
and roll Skia continuously. Using unconfigured opt-in defines leaves new code
disabled across Skia's CI bots, preventing tests from validating the fix or
feature before rolling into client repositories. Inverted opt-out guards
(SK_IGNORE_*, SK_DISABLE_*, or SK_USE_LEGACY_*) ensure Skia validates the new
code path by default while allowing downstream clients to temporarily define the
opt-out flag to avoid breakages during migration.

## Examples

Bad:

```cpp
// BUILD.gn: Flag omitted in Skia; new code unexercised in Skia CI
optional("png_encode") {
  public_defines = [ "SK_CODEC_ENCODES_PNG" ]
}

// C++: Opt-in define leaves new code unexercised by default in Skia builds
#ifdef SK_USE_MAD_IN_RASTER_PIPELINE
    F dither = mad(cast(M), 2/128.0f, -63/128.0f);
#else
    F dither = cast(M) * (2/128.0f) - (63/128.0f);
#endif

#ifdef SK_SKPARAGRAPH_ELLIPSIS_BEHAVIOR
    if (!fTooLongWord && !hasEllipsis) {  // New behavior opt-in
#else
    if (!fTooLongWord || hasEllipsis) {   // Old behavior default
#endif
```

Good:

```cpp
// BUILD.gn: Flag defined in Skia so tests exercise new code immediately
optional("png_encode") {
  public_defines = [
    "SK_CODEC_ENCODES_PNG",
    "SK_PNG_ENCODER_USE_CONVERT_PIXELS",
  ]
}

// C++: Inverted opt-out guard enables new code by default in Skia builds
#ifndef SK_DISABLE_MAD_IN_RASTER_PIPELINE
    F dither = mad(cast(M), 2/128.0f, -63/128.0f);
#else
    F dither = cast(M) * (2/128.0f) - (63/128.0f);
#endif

#ifdef SK_IGNORE_SKPARAGRAPH_ELLIPSIS_FIX
    if (!fTooLongWord || hasEllipsis) {   // Old behavior opt-out
#else
    if (!fTooLongWord && !hasEllipsis) {  // New behavior default
#endif
```
