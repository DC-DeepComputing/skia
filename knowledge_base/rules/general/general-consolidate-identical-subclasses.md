---
name: skia/general-consolidate-identical-subclasses
description: When defining derived classes for different semantic roles, avoid creating multiple subclasses with identical fields, methods, and lifecycle behavior; consolidate them into one reusable class
rubric: design
severity: should_fix
---

**Rule.** Do not create multiple distinct subclasses solely to represent
different caller contexts or AST roles when their member fields, methods, and
lifecycle management are identical. Consolidate them into a single reusable
derived class, or use type aliases if context-specific naming is desired.

**Why.** Defining duplicate subclasses that implement identical functionality
creates boilerplate code and artificial class hierarchies without behavioral
differentiation. Consolidating identical subclasses into a single class reduces
maintenance overhead and avoids over-engineering.

## Examples

Bad:

```cpp
class FieldLValue final : public UnownedLValueSlice {
public:
    explicit FieldLValue(std::unique_ptr<LValue> p, int initialSlot, int numSlots)
            : UnownedLValueSlice(p.release(), initialSlot, numSlots) {}
    ~FieldLValue() override { delete fParent; }
};

class SliceableSwizzleLValue final : public UnownedLValueSlice {
public:
    explicit SliceableSwizzleLValue(std::unique_ptr<LValue> p, int initialSlot, int numSlots)
            : UnownedLValueSlice(p.release(), initialSlot, numSlots) {}
    ~SliceableSwizzleLValue() override { delete fParent; }
};
```

Good:

```cpp
class LValueSlice final : public UnownedLValueSlice {
public:
    explicit LValueSlice(std::unique_ptr<LValue> p, int initialSlot, int numSlots)
            : UnownedLValueSlice(p.release(), initialSlot, numSlots) {}
    ~LValueSlice() override { delete fParent; }
};
```
