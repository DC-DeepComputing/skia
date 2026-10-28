---
name: skia/sksl/delete-unused-sksl-module-helpers
description: When refactoring SkSL module files, delete orphaned internal helper functions and their supporting constants when callers are eliminated, and regenerate module assets
rubric: readability
severity: nit
---

**Rule.** When refactoring SkSL module files (`src/sksl/*.sksl`) eliminates
callers of internal helper functions (such as `$`-prefixed functions), delete
the dead function declarations, implementations, and any companion constants or
enum values defined solely for them. Regenerate the compiled module assets after
removing dead code so built-in tables remain minimal.

**Why.** In SkSL module files, functions and constants are compiled into
built-in module tables bundled into the binary as library symbols. Because
module symbols are treated as externally visible library entry points, compilers
do not emit unused-symbol diagnostics when callers within the module are
eliminated, allowing orphaned helpers and supporting constants to bloat module
assets undetected.

## Examples

Bad:

```cpp
// Callers eliminated during refactoring, but function and supporting constants remain:
const int $kReadSwizzleNormalRGBA = 0;
const int $kReadSwizzleRGB1 = 1;
$pure half4 $apply_swizzle(int swizzleType, half4 color) { ... }

// Orphaned helper left in module:
$pure float3x3 $map_to_unit_x(float2 p0, float2 p1) {
    return float3x3(...) * inverse(...);
}
```

Good:

```cpp
// Delete obsolete helpers ($map_to_unit_x, $apply_swizzle) and their dedicated
// supporting constants ($kReadSwizzle*) from the SkSL module when callers are
// refactored away, and re-generate the compiled module assets.
```
