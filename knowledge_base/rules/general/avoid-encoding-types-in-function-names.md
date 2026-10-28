---
name: skia/avoid-encoding-types-in-function-names
description: When naming helper functions or math utilities, avoid encoding parameter types, return types, or wrapper names; use descriptive action names and C++ overloads
rubric: readability
severity: should_fix
---

**Rule.** Do not encode concrete parameter types, return types, or transient
wrapper types (such as `_special`, `Scalar`, or `Float`) into function names.
Name helper and utility functions after the action, concept, or algorithm they
execute rather than the types they manipulate, and provide type-agnostic names
with C++ function overloads or templates when operating across multiple types
(such as `SkIsFinite` for float and double).

**Why.** Embedding type names or wrapper types into function names clutters
identifiers, creates refactoring churn when types evolve, and makes call sites
misleading or prone to unintended precision loss when invoked with other types.
Naming functions after the algorithm and leveraging C++ function overloads keeps
interfaces clean, type-safe, and unified across types.

## Examples

Bad:

```cpp
// Bad: Encodes return type / transitional wrapper 'special' into helper name
template <typename T>
static sk_sp<SkSpecialImage> make_blurred_special(PassMaker* makerX, PassMaker* makerY,
                                                  SkBitmap src, const SkIRect& originalSrcBounds,
                                                  const SkIRect& originalDstBounds,
                                                  SkArenaAlloc* alloc);

// Bad: Encodes parameter type 'Scalar' into function name; misleading with double
static inline bool SkScalarIsFinite(SkScalar x) { return (x - x) == 0; }
static inline bool SkScalarsAreFinite(SkScalar a, SkScalar b) { return (a - a) == (b - b); }
double a = fA, b = fB, c = fC;
if (!SkScalarsAreFinite(a, b) || !SkScalarIsFinite(c)) { ... }
```

Good:

```cpp
// Good: Names function after the operation and algorithm it executes
template <typename T>
static sk_sp<SkSpecialImage> eval_blur_passes(PassMaker* makerX, PassMaker* makerY,
                                              SkBitmap src, const SkIRect& originalSrcBounds,
                                              const SkIRect& originalDstBounds,
                                              SkArenaAlloc* alloc);

// Good: Uses type-agnostic name with C++ overloads for float and double
static inline bool SkIsFinite(float x) { return (x - x) == 0; }
static inline bool SkIsFinite(double x) { return (x - x) == 0; }
static inline bool SkIsFinite(float a, float b) { return (a - a) == (b - b); }
static inline bool SkIsFinite(double a, double b) { return (a - a) == (b - b); }
double a = fA, b = fB, c = fC;
if (!SkIsFinite(a, b) || !SkIsFinite(c)) { ... }
```
