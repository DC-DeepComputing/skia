---
name: skia/annotate-preprocessor-closing-directives
description: 'When writing preprocessor conditionals or header include guards, annotate closing #else and #endif directives with trailing comments naming the guarded macro or condition'
rubric: readability
severity: nit
---

**Rule.** When writing preprocessor conditional blocks (#if, #ifdef, #ifndef)
that span multi-line spans or contain nested directives—including header include
guards (#ifndef <Header>_DEFINED)—annotate closing #else and #endif directives
with a trailing comment identifying the condition or macro name (e.g., '#else //
MACRO', '#endif // MACRO', or '#endif // <Header>_DEFINED'). Omit comments only
on short, self-contained conditional blocks where the opening condition is
immediately visible without scrolling.

**Why.** When conditional compilation blocks span dozens of lines, cross
function boundaries, or enclose entire header files, annotating closing #else
and #endif directives clarifies which feature toggle or guard is terminating
without requiring readers to scroll back up. Omit annotations on short blocks to
avoid redundant visual clutter.

## Examples

Bad:

```cpp
#ifndef SkBlitter_A8_DEFINED
#define SkBlitter_A8_DEFINED

// declarations...

#endif

#if defined(SK_ENABLE_OPTIMIZE_SIZE)
// 50+ lines of size-optimized implementation code
#else
// 50+ lines of unconstrained implementation code
#endif
```

Good:

```cpp
#ifndef SkBlitter_A8_DEFINED
#define SkBlitter_A8_DEFINED

// declarations...

#endif  // SkBlitter_A8_DEFINED

#if defined(SK_ENABLE_OPTIMIZE_SIZE)
// 50+ lines of size-optimized implementation code
#else  // !defined(SK_ENABLE_OPTIMIZE_SIZE)
// 50+ lines of unconstrained implementation code
#endif  // SK_ENABLE_OPTIMIZE_SIZE
```
