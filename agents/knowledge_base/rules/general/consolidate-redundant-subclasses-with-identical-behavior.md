---
name: skia/consolidate-redundant-subclasses-with-identical-behavior
description: When defining subclasses sharing identical state and behavior without polymorphic overrides, consolidate them into a single parameterized class configured at call sites
rubric: design
severity: should_fix
---

**Rule.** Do not introduce separate subclasses or deep inheritance hierarchies
when multiple derived variants share identical fields, methods, and destruction
behavior. Consolidate identical subclasses into a single class parameterized by
its inputs or configured at construction, computing variant-specific parameters
(such as offsets, slot counts, or indices) at caller sites.

**Why.** Creating separate subclasses that do not override virtual methods or
introduce unique state increases inheritance depth and class proliferation
without polymorphic benefit. Consolidating identical variants into a single
parameterized class simplifies the type hierarchy, eliminates boilerplate
destructors and constructors, and improves maintainability.

## Examples

Bad:

```cpp
class FixedIndexLValue final : public UnownedLValueSlice {
public:
    explicit FixedIndexLValue(std::unique_ptr<LValue> p, int indexValue, const Type& indexedType)
            : UnownedLValueSlice(p.release(),
                                 indexedType.slotCount() * indexValue,
                                 indexedType.slotCount()) {}
    ~FixedIndexLValue() override { delete fParent; }
};

class FieldLValue final : public UnownedLValueSlice {
public:
    explicit FieldLValue(std::unique_ptr<LValue> p, int initialSlot, int numSlots)
            : UnownedLValueSlice(p.release(), initialSlot, numSlots) {}
    ~FieldLValue() override { delete fParent; }
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

// Call sites compute slice offset and slot count:
return std::make_unique<LValueSlice>(std::move(base), fieldExpr.initialSlot(),
                                     fieldExpr.type().slotCount());
```
