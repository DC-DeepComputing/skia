---
name: skia/avoid-redundant-compiler-abstraction-guards
description: 'When suppressing sanitizers or applying compiler attributes, use dedicated SkAttributes.h macros (like SK_NO_SANITIZE_CFI) instead of bare SK_NO_SANITIZE("cfi") or redundant #if guards'
rubric: correctness
severity: blocking
---

**Rule.** Use dedicated compiler abstraction macros in SkAttributes.h (such as
SK_NO_SANITIZE_CFI) rather than raw __attribute__ directives, ad-hoc #if
defined(__clang__) guards, or bare SK_NO_SANITIZE("cfi"). When applying compiler
abstraction macros (such as SK_NO_SANITIZE_CFI, SK_ALWAYS_INLINE, or
SK_TRIVIAL_ABI), use them directly at call sites without redundant manual #if
checks, as their definitions already encapsulate toolchain detection and safely
expand to no-ops on unsupported targets.

**Why.** GCC supports __has_attribute(no_sanitize) but warns on unrecognized
sanitizer strings like "cfi". While internal builds suppress attribute warnings,
downstream embedders compile headers with -Werror and without -Wno-attributes,
failing GCC builds. Centralizing toolchain detection in SkAttributes.h protects
downstream builds, avoids preprocessor clutter, and eliminates redundant #if
checks at call sites.

## Examples

Bad:

```cpp
// Bad: bare SK_NO_SANITIZE with compiler-specific sanitizer string breaks GCC embedders
SK_NO_SANITIZE("cfi")
static T* TCast(void* buffer) { return (T*)buffer; }

// Bad: redundant manual preprocessor check around abstraction macro
#if defined(__clang__)
SK_NO_SANITIZE_CFI
#endif
inline void invoke() { /* ... */ }
```

Good:

```cpp
// Good: use dedicated macro in SkAttributes.h, which no-ops on GCC
SK_NO_SANITIZE_CFI
static T* TCast(void* buffer) { return (T*)buffer; }

// Good: use abstraction macro directly without manual #if checks
SK_NO_SANITIZE_CFI
inline void invoke() { /* ... */ }
```
