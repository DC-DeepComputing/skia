---
name: skia/combine-identical-preprocessor-branches
description: When preprocessor blocks share identical conditions or branches share identical bodies, combine conditions using logical OR or fuse adjacent blocks
rubric: readability
severity: nit
---

**Rule.** When preprocessor conditional branches (`#if` or `#elif`) share
identical macro definitions, bodies, or comments, combine their conditions using
logical OR (`||`) into a single conditional block. Similarly, when adjacent
`#if` or `#ifdef` blocks share the exact same preprocessor guard, fuse them into
a single conditional block rather than closing and immediately reopening
identical directives.

**Why.** Duplicating preprocessor branches with identical macro definitions or
settings creates redundant boilerplate and increases maintenance overhead.
Similarly, closing and immediately reopening identical preprocessor conditions
fragments related declarations and adds visual clutter; consolidating them keeps
configuration handling concise and cleanly grouped.

## Examples

Bad:

```cpp
// Bad: Duplicating identical preprocessor bodies across branches
#elif defined(__x86_64__) || defined(SK_CPU_ARM64)
    #define ABI
    #define JUMPER_NARROW_STAGES 0
#elif defined(SK_CPU_LOONGARCH)
    #define ABI
    #define JUMPER_NARROW_STAGES 0

// Bad: Closing and immediately reopening identical preprocessor guards
#if defined(SK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE)
void SkDraw::drawDevMask(const SkMask& srcM, const SkPaint& paint) const { ... }
#endif

#if defined(SK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE)
void SkDraw::drawBitmapAsMask(const SkBitmap& bitmap, ...) const { ... }
#endif
```

Good:

```cpp
// Good: Combine conditions using logical OR (||)
#elif defined(__x86_64__) || defined(SK_CPU_ARM64) || defined(SK_CPU_LOONGARCH)
    #define ABI
    #define JUMPER_NARROW_STAGES 0

// Good: Fuse adjacent blocks under a single preprocessor guard
#if defined(SK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE)
void SkDraw::drawDevMask(const SkMask& srcM, const SkPaint& paint) const { ... }

void SkDraw::drawBitmapAsMask(const SkBitmap& bitmap, ...) const { ... }
#endif
```
