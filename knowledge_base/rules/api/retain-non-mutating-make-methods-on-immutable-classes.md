---
name: skia/api/retain-non-mutating-make-methods-on-immutable-classes
description: When designing or migrating classes to be immutable, retain non-mutating make* copy-transformation methods in public APIs rather than hiding them or requiring builders
rubric: design
severity: should_fix
---

**Rule.** When designing or migrating classes to be immutable value types (such
as geometric paths), retain non-mutating make* copy-transformation methods (such
as makeTransform, makeOffset, or makeScale) in the public API. Do not hide or
remove them under feature flags or force callers to instantiate an intermediate
builder object for simple transformations.

**Why.** Non-mutating make* transformation methods return a new instance by
value without mutating the receiver, preserving immutability invariants.
Retaining them preserves caller ergonomics, avoids intermediate builder
overhead, and enables single-allocation optimizations.

## Examples

Bad:

```cpp
#ifdef SK_HIDE_PATH_EDIT_METHODS
private:
#endif
    SkPath makeTransform(const SkMatrix& m,
                         SkApplyPerspectiveClip pc = SkApplyPerspectiveClip::kYes) const;
    SkPath makeOffset(SkScalar dx, SkScalar dy) const;
    SkPath makeScale(SkScalar sx, SkScalar sy) const;
```

Good:

```cpp
public:
    SkPath makeOffset(SkScalar dx, SkScalar dy) const {
        SkPath dst;
        this->offset(dx, dy, &dst);
        return dst;
    }

    SkPath makeTransform(const SkMatrix& m,
                         SkApplyPerspectiveClip pc = SkApplyPerspectiveClip::kYes) const {
        SkPath dst;
        this->transform(m, &dst, pc);
        return dst;
    }

    SkPath makeScale(SkScalar sx, SkScalar sy) const {
        return this->makeTransform(SkMatrix::Scale(sx, sy), SkApplyPerspectiveClip::kNo);
    }
```
