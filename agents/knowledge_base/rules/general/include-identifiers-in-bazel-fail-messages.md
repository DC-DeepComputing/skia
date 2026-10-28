---
name: skia/include-identifiers-in-bazel-fail-messages
description: When raising build errors with fail() in Bazel or Starlark files, include the offending target, dependency, or argument value in the error message
rubric: readability
severity: nit
---

**Rule.** When raising build errors using fail(...) in Bazel and Starlark macros
or rules (*.bzl), include the offending identifier, target, dependency label, or
invalid value in the error message instead of using a generic static string. Use
string concatenation, % formatting, or .format() to identify the exact
problematic input.

**Why.** Static error messages like fail("not supported") or fail("unknown CPU")
leave developers without context when a build fails during loading or analysis.
Interpolating the offending dependency label, target name, or configuration
argument pinpoints the problematic input immediately in build logs.

## Examples

Bad:

```cpp
for dep in implementation_deps:
    if dep in _REWRITE_TEST_UTIL_DEPS:
        fail("not supported")
```

Good:

```cpp
for dep in implementation_deps:
    if dep in _REWRITE_TEST_UTIL_DEPS:
        fail("not supported " + dep)
```
