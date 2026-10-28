---
name: skia/modules/submit-skcms-changes-to-standalone-repo
description: When modifying code under modules/skcms/, do not edit files directly in Skia; submit changes to the upstream standalone skcms repository instead
rubric: design
severity: blocking
---

**Rule.** Do not modify skcms source or header files in modules/skcms/ directly
in a Skia repository CL. Submit changes to the upstream standalone skcms
repository (https://skia.googlesource.com/skcms/) so they roll into Skia
automatically via the autoroller.

**Why.** SkCMS is maintained in a standalone repository and vendored into Skia
via an automated roller. Making direct edits to modules/skcms/ in a Skia CL
bypasses the upstream source of truth, causing conflicts or getting overwritten
when the next skcms roll lands.

## Examples

Bad:

```cpp
// Directly modifying modules/skcms/ source or header files in a Skia CL:
// modules/skcms/src/skcms_internals.h
#if __has_cpp_attribute(clang::musttail) && !defined(__powerpc__)
    #define SKCMS_HAS_MUSTTAIL 1
#endif

// modules/skcms/src/Transform_inl.h
return __builtin_convertvector((I16)half, F);
```

Good:

```cpp
// Revert or omit direct edits to modules/skcms/ from the Skia CL.
// Instead, submit changes upstream to the standalone skcms repository:
// https://skia.googlesource.com/skcms/ (review: https://skia-review.googlesource.com/c/skcms/)
// The changes will roll into Skia automatically via the Autoroll bot.
```
