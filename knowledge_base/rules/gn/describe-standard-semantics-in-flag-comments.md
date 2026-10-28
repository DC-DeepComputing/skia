---
name: skia/gn/describe-standard-semantics-in-flag-comments
description: When documenting compiler flags that alter language semantics in build files, explain standard language rules accurately rather than framing standard compliance as an optional compiler optimization
rubric: readability
severity: should_fix
---

**Rule.** When documenting compiler flags that relax or enforce standard
language access rules (such as `-fstrict-aliasing` vs `-fno-strict-aliasing`),
accurately describe standard C/C++ semantics. Explain that standard compliance
(such as strict aliasing and glvalue access rules) is standard language behavior
and that violating it is undefined behavior, whereas disabling flags like
`-fno-strict-aliasing` enables a non-standard dialect for code assuming
arbitrary pointer aliasing.

**Why.** Inaccurate comments misinform developers into treating undefined
behavior as merely a missing compiler optimization. Clarifying standard
semantics helps contributors understand why flags like `-fno-strict-aliasing`
are necessary when embedding projects that violate standard aliasing rules.

## Examples

Bad:

```cpp
# Strict aliasing is a compiler optimization that assumes pointers to different
# types never alias. This can lead to undefined behavior if the pointers do.
config("strict_aliasing") {
  cflags = [ "-fstrict-aliasing" ]
}
```

Good:

```cpp
# PartitionAlloc, Chrome, and many other C++ projects are written assuming
# different types may freely point to the same location in memory. This is
# undefined behavior in C/C++. With `-fstrict-aliasing`, some compilers will
# optimize code in a way that breaks those projects.
config("strict_aliasing") {
  cflags = [ "-fstrict-aliasing" ]
}
```
