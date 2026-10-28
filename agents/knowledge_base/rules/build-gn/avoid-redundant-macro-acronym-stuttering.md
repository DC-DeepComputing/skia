---
name: skia/build-gn/avoid-redundant-macro-acronym-stuttering
description: When naming preprocessor macros or build feature defines, avoid redundant acronym stuttering or prefix expansions when an acronym token already encapsulates that prefix
rubric: readability
severity: should_fix
---

**Rule.** In preprocessor macros and build feature defines (such as in BUILD.gn,
Bazel, or public headers), do not duplicate an expanded prefix before an acronym
that already encapsulates it. Name availability and feature macros concisely
without redundant stuttering (e.g., prefer SK_FONTMGR_FCI_AVAILABLE over
SK_FONTMGR_FONTCONFIG_FCI_AVAILABLE).

**Why.** Including both an expanded name and its abbreviation (such as
FONTCONFIG and FCI, where FCI already stands for FontConfigInterface) creates
redundant acronym stuttering. Concise, non-redundant macro identifiers keep
build definitions readable and maintain consistent naming patterns across
feature toggles.

## Examples

Bad:

```cpp
optional("fontmgr_fci") {
  public_defines = [ "SK_FONTMGR_FONTCONFIG_FCI_AVAILABLE" ]
}
```

Good:

```cpp
optional("fontmgr_fci") {
  public_defines = [ "SK_FONTMGR_FCI_AVAILABLE" ]
}
```
