---
name: skia/sksl/assert-sized-array-before-unrolling
description: When asserting preconditions on array types before unrolling or iterating over elements in SkSL code generation, assert that the array is sized using SkASSERT(!type.isUnsizedArray())
rubric: correctness
severity: nit
---

**Rule.** When asserting preconditions on array types before unrolling,
indexing, or iterating over elements in SkSL backend code generators, verify
that the type is bounded by asserting !type.isUnsizedArray() (or type.isArray()
&& !type.isUnsizedArray()). Do not accidentally assert the unnegated predicate
type.isUnsizedArray(), which inverts the invariant and causes debug assertions
to fail on valid sized arrays while failing to catch invalid unbounded arrays.

**Why.** In SkSL, unsized arrays have unbounded element counts and cannot be
statically indexed or unrolled via .columns(). Omitting the negation in the
assertion inverts the check, causing debug builds to crash on valid sized arrays
while failing to catch invalid unbounded arrays before iteration.

## Examples

Bad:

```cpp
// Bad: Inverted assertion causes debug crashes on valid sized arrays.
SkASSERT(ctor.type().isUnsizedArray());
for (int i = 0; i < ctor.type().columns(); ++i) {
    // ...
}
```

Good:

```cpp
// Good: Asserts that the array is bounded before unrolling elements.
SkASSERT(!ctor.type().isUnsizedArray());
for (int i = 0; i < ctor.type().columns(); ++i) {
    // ...
}
```
