---
name: skia/sksl/prefix-internal-sksl-module-helpers-with-dollar
description: When defining internal helper functions or constants in SkSL module files, prefix their names with $ to mark them private and enable compiler optimizations
rubric: readability
severity: should_fix
---

**Rule.** In SkSL module files (`src/sksl/*.sksl`), preface internal helper
functions and module-scoped constants with `$` (e.g., `$apply_swizzle`,
`$kDefaultColor`) unless they are intended to be externally accessible library
symbols. Do not define unprefixed helper functions or constants for
module-internal implementation details.

**Why.** In SkSL modules, functions and constants without a `$` prefix are
treated as externally accessible library symbols. Prefixing private helpers and
constants with `$` designates them as internal to SkSL, enabling compiler passes
such as symbol minification (SkSLRenamePrivateSymbols) and dead-code elimination
(SkSLEliminateDeadGlobalVariables) while avoiding name collisions with
user-defined shaders.

## Examples

Bad:

```cpp
// Bad: Unprefixed helper function is treated as a public module symbol.
$pure half4 apply_swizzle(int swizzleType, half4 color) {
    return (swizzleType == 1) ? color.bgra : color.rgba;
}

const half4 kDefaultColor = half4(1, 0, 0, 1);
```

Good:

```cpp
// Good: Prefixing with $ marks the function and constant as private/internal.
$pure half4 $apply_swizzle(int swizzleType, half4 color) {
    return (swizzleType == 1) ? color.bgra : color.rgba;
}

const half4 $kDefaultColor = half4(1, 0, 0, 1);
```
