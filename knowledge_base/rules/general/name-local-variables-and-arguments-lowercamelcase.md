---
name: skia/name-local-variables-and-arguments-lowercamelcase
description: When declaring local variables, structured bindings, or function arguments, name them using lowerCamelCase without member prefix 'f', snake_case, or UpperCamelCase
rubric: readability
severity: should_fix
---

**Rule.** Name local variables, structured bindings, and function arguments
using lowerCamelCase (camel-capped with no initial capital) rather than
snake_case, UpperCamelCase, or member-field prefixes. Do not prefix local
variables or arguments with 'f' (which is strictly reserved for class and struct
member fields), and do not use underscore separators unless conforming to an
external interface.

**Why.** Skia style specifies lowerCamelCase for local variables, structured
bindings, and arguments to distinguish them from static non-class helper
functions (which use snake_case), compile-time constants (which use a 'k'
prefix), and member fields (which use an 'f' prefix). Prefixing local variables
with 'f' misleadingly implies instance state access at the point of use.

## Examples

Bad:

```cpp
// Bad: snake_case, UpperCamelCase, or member prefix 'f' used for local variables and arguments.
void processItems(int item_count) {
    const bool fProgressive = true;
    bool should_grow = false;
    int64_t max_abs_value = 0;
    int CurrentIndex = 0;
    std::array<float, 4> sub_x;
    auto [rs_udb, combined_tdb] = gatherer.endRenderStepData(/*performsShading=*/true);
    UniformDataBlock paint_udb = gatherer.endPaintData();
}
```

Good:

```cpp
// Good: lowerCamelCase without 'f' prefix used for local variables and arguments.
void processItems(int itemCount) {
    const bool isProgressive = true;
    bool shouldGrow = false;
    int64_t maxAbsValue = 0;
    int currentIndex = 0;
    std::array<float, 4> subX;
    auto [rsUdb, combinedTdb] = gatherer.endRenderStepData(/*performsShading=*/true);
    UniformDataBlock paintUdb = gatherer.endPaintData();
}
```
