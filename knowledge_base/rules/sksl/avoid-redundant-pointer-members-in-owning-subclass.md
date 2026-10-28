---
name: skia/sksl/avoid-redundant-pointer-members-in-owning-subclass
description: When an owning derived class inherits from a non-owning base holding a raw pointer, avoid storing a duplicate pointer member to the same object
rubric: safety
severity: nit
---

**Rule.** Do not store redundant pointer members to the same target object
across base and derived classes (such as adding a std::unique_ptr in a derived
class when the base class already stores a raw pointer to that object). Reuse
the base class pointer and manage its destruction in the derived class or hoist
ownership into the base to eliminate layout bloat.

**Why.** Storing redundant pointers to the same target object across base and
derived classes inflates object layout size for every instance. Reusing the base
pointer or restructuring ownership eliminates unnecessary memory overhead
without altering lifetime semantics.

## Examples

Bad:

```cpp
class FieldLValue final : public UnownedFieldLValue {
public:
    explicit FieldLValue(std::unique_ptr<LValue> p, int i, int n)
            : UnownedFieldLValue(p.get(), i, n)
            , fOwnedParent(std::move(p)) {}

private:
    std::unique_ptr<LValue> fOwnedParent;
};
```

Good:

```cpp
class FieldLValue final : public UnownedFieldLValue {
public:
    explicit FieldLValue(std::unique_ptr<LValue> p, int i, int n)
            : UnownedFieldLValue(p.release(), i, n) {}

    ~FieldLValue() override {
        delete fParent;
    }
};
```
