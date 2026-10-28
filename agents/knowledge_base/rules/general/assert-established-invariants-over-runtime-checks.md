---
name: skia/assert-established-invariants-over-runtime-checks
description: When prior validation, class invariants, or constructor initialization guarantee valid state or non-null pointers, access directly or assert with SkASSERT/.value() instead of redundant checks
rubric: readability
severity: should_fix
---

**Rule.** When prior validation, factory logic, constructor initialization,
class invariants, preceding assertions, or loop bounds guarantee that an
operation cannot fail, a member or pointer is non-null, or an index is within
bounds, access the pointer directly, assert the invariant using SkASSERT (or
SkASSERT_RELEASE), or unpack optional values directly with .value(). Do not add
redundant defensive runtime checks (such as if (fMember) or if (!ptr)), bounds
checks, error logs, clamping operations (such as SkTPin), or conditional
fallback branches for impossible failure conditions; access guaranteed members
directly or assert invariants with SkASSERT rather than branching with if
(condition) { ... } else { SK_ABORT(...); }.

**Why.** Defensive runtime checks, dead bounds checks inside loops, clamping
operations, or if/else SK_ABORT branches for impossible failures clutter control
flow with unnecessary nesting and unreachable dead branches, and mislead
maintainers into expecting invalid states. Accessing guaranteed members directly
or asserting invariants via SkASSERT or SkASSERT_RELEASE documents contracts and
catches violations in debug/release builds while eliminating redundant runtime
branching and flattening control flow.

## Examples

Bad:

```cpp
// Bad: Redundant runtime null check on member guaranteed by constructor
if (this->fSlotManager) {
    this->fSlotManager->fSlotsRoot = jroot["slots"];
}

// Bad: Redundant runtime error check when anim->size() was already validated
const auto scale_matrix = SkMatrix::Rect2Rect(SkRect::MakeSize(anim->size()),
                                              SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                              SkMatrix::kCenter_ScaleToFit);
if (!scale_matrix) {
    return 1;
}

// Bad: Branching with if/else SK_ABORT on an internal invariant
if (v->storage() == Variable::Storage::kParameter) {
    expr += this->assembleName(v->mangledName());
} else {
    SK_ABORT("Unsized array variable reference must be a parameter.");
}
```

Good:

```cpp
// Good: Access member directly since constructor initialization guarantees presence
this->fSlotManager->fSlotsRoot = jroot["slots"];

// Good: anim->size() is guaranteed non-empty by construction; unpack directly with .value()
const SkMatrix scale_matrix = SkMatrix::Rect2Rect(SkRect::MakeSize(anim->size()),
                                                  SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                  SkMatrix::kCenter_ScaleToFit).value();

// Good: Assert invariant directly with SkASSERT to flatten control flow
SkASSERT(v->storage() == Variable::Storage::kParameter);
expr += this->assembleName(v->mangledName());
```
