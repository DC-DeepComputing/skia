---
name: skia/api/keep-experimental-algorithms-external
description: When developing experimental or prototype algorithms on core classes like SkPath, stage them as external standalone utilities instead of adding or modifying core methods
rubric: design
severity: should_fix
---

**Rule.** Do not add experimental or prototype algorithms as member methods to
core public API classes (such as SkPath) or directly alter established core
methods to incorporate them. Instead, stage experimental helpers and prototype
algorithms as external standalone utility functions (e.g., in tools/viewer
slides or dedicated utility modules like SkPathUtils) until they are mature and
stabilized.

**Why.** Modifying core public classes or altering established methods with
experimental algorithms expands the public interface, risks behavioral
regressions and churn for downstream embedders, and creates long-term API
maintenance burdens. Staging prototype algorithms as standalone external
utilities keeps core class interfaces minimal, isolates risk, and allows
experimental code to evolve freely.

## Examples

Bad:

```cpp
// include/core/SkPath.h or src/core/SkPath.cpp: adding or modifying core member methods
class SK_API SkPath {
public:
    bool general_interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;
};

bool SkPath::interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
    // Directly modifying established core method with experimental algorithm
    createPathFromTValues(*this, tValuesToAdd1, tValues1, &thisCubic);
    createPathFromTValues(ending, tValuesToAdd2, tValues2, &endingCubic);
    ...
}
```

Good:

```cpp
// tools/viewer/PathLerpSlide.cpp or dedicated utility header (e.g. include/utils/SkPathUtils.h):
// Stage experimental algorithm as external standalone utility function
bool generalInterpolate(const SkPath& beginning, const SkPath& ending, SkScalar weight, SkPath* out) {
    if (beginning.isInterpolatable(ending)) {
        return beginning.interpolate(ending, weight, out);
    }
    // Prototype general interpolation isolated in utility/tool code
    ...
}
```
