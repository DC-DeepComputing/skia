---
name: skia/order-global-before-namespaced-forward-declarations
description: When adding or modifying forward declarations in C++ headers, keep declarations in alphabetical order and place global declarations before namespaced blocks
rubric: readability
severity: nit
---

**Rule.** In C++ header files, maintain alphabetical order for forward
declarations of classes, structs, and enums when adding, updating, or renaming
types. Additionally, place top-level (global namespace) forward declarations
before any namespaced forward-declaration blocks, keeping declarations sorted
alphabetically within each section, and avoid placing top-level declarations
below namespace blocks.

**Why.** Alphabetical sorting makes forward declarations predictable to locate,
helps avoid duplicate declarations, and adheres to Skia coding style guidelines.
Placing global forward declarations before namespaced blocks prevents ambiguous
scoping where readers might misinterpret top-level declarations as belonging to
the preceding namespace.

## Examples

Bad:

```cpp
// Out of alphabetical order when adding or renaming:
class SkSurface;
class GrSurfaceCharacterization;
enum GrSurfaceOrigin : int;

// Global declarations placed below namespaced blocks:
namespace skcpu {
class Recorder;
}
class SkCanvas;
```

Good:

```cpp
// Alphabetically sorted forward declarations:
class GrSurfaceCharacterization;
enum GrSurfaceOrigin : int;
class SkCanvas;
class SkSurface;

// Global declarations placed before namespaced blocks:
namespace skcpu {
class Recorder;
}
```
