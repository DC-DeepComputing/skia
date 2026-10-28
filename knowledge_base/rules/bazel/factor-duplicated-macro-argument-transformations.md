---
name: skia/bazel/factor-duplicated-macro-argument-transformations
description: When transforming target arguments across multiple Bazel/Starlark wrapper macros, factor duplicated list-processing loops into a shared private helper function
rubric: design
severity: nit
---

**Rule.** In Bazel/Starlark macro files, factor duplicated argument
transformation logic (such as dependency rewriting, flag manipulation, or list
filtering) into a shared private helper function. Do not duplicate
element-transformation loops or dictionary lookup logic across sibling macro
wrappers.

**Why.** Repeating argument-transformation loops across multiple rule wrappers
introduces code bloat and creates a maintenance hazard where changes to lookup
tables or transformation logic risk diverging across macros. Extracting common
transformations into a private helper keeps macros concise and guarantees
consistent target definitions.

## Examples

Bad:

```cpp
# In skia_cc_library_with_testutil:
rewritten_deps = []
for dep in deps:
    if dep in _REWRITE_TEST_UTIL_DEPS:
        rewritten_deps.append(_REWRITE_TEST_UTIL_DEPS[dep])
    else:
        rewritten_deps.append(dep)

# In skia_objc_library_with_testutil:
rewritten_deps = []
for dep in deps:
    if dep in _REWRITE_TEST_UTIL_DEPS:
        rewritten_deps.append(_REWRITE_TEST_UTIL_DEPS[dep])
    else:
        rewritten_deps.append(dep)
```

Good:

```cpp
def _rewrite_deps(deps):
    rewritten_deps = []
    for dep in deps:
        if dep in _REWRITE_TEST_UTIL_DEPS:
            rewritten_deps.append(_REWRITE_TEST_UTIL_DEPS[dep])
        else:
            rewritten_deps.append(dep)
    return rewritten_deps

# In skia_cc_library_with_testutil:
rewritten_deps = _rewrite_deps(deps)

# In skia_objc_library_with_testutil:
rewritten_deps = _rewrite_deps(deps)
```
