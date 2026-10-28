---
name: skia/describe-standard-template-semantics-in-comments
description: When documenting template idioms or workarounds in code comments, accurately describe standard language semantics rather than speculating about compiler bugs
rubric: readability
severity: nit
---

**Rule.** When documenting C++ template idioms, workarounds, or compile-time
evaluation guards in code comments, accurately describe standard language
semantics (such as non-dependent static_assert expressions failing during
definition parsing before template instantiation) rather than speculating about
compiler bugs, quirks, or non-standard evaluation.

**Why.** Speculating that standard C++ template lookup or instantiation rules
are compiler-specific quirks or bugs misleads maintainers about language
mechanics and standards compliance. Accurately explaining standard template
semantics clarifies why dependent expressions (such as !sizeof(T)) are necessary
and prevents churn when upgrading toolchains or standard versions.

## Examples

Bad:

```cpp
// This is really an assert(false), but if it doesn't reference T, some compilers seem to
// evaluate it universally (and then fail).
static_assert(!sizeof(T), "Unsupported type for trace argument");
```

Good:

```cpp
// This is really an assert(false), but if it doesn't reference T, the static_assert fails
// before the template is instantiated.
static_assert(!sizeof(T), "Unsupported type for trace argument");
```
