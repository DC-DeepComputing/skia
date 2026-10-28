---
name: skia/modules/wrap-optional-collections-in-sk-sp
description: When storing optional collections in copyable value types, wrap the collection in an sk_sp ref-counted struct instead of direct embedding or std::unique_ptr
rubric: safety
severity: should_fix
---

**Rule.** When a lightweight, copyable value type optionally holds a collection
or dynamic array, wrap the collection inside an SkNVRefCnt struct managed by
sk_sp rather than embedding the container directly or using std::unique_ptr.
Avoid hand-rolling custom dynamic array allocations or bloating value types with
rarely used inline storage.

**Why.** Directly embedding containers like std::vector or STArray bloats value
types even when empty and forces expensive deep copies on every assignment.
Using std::unique_ptr avoids bloat but deletes copy operations, whereas wrapping
the container in an sk_sp-managed ref-counted struct maintains O(1) copyability
and keeps the empty object size down to a single pointer.

## Examples

Bad:

```cpp
// 1. Direct container bloats the value object and forces deep copies:
struct SkSVGColor {
    Type fType;
    SkSVGColorType fColor;
    std::vector<SkString> fVars;
};

// 2. std::unique_ptr prevents copy construction/assignment:
struct SkSVGColor {
    Type fType;
    SkSVGColorType fColor;
    std::unique_ptr<std::vector<SkString>> fVars;
};
```

Good:

```cpp
// Lightweight copyable value type holding an optional collection via sk_sp:
struct SkSVGColor {
    Type fType;
    SkSVGColorType fColor;

    using Vars = std::vector<SkString>;
    struct RefCntVars : public SkNVRefCnt<RefCntVars> {
        RefCntVars(Vars&& vars) : fData(std::move(vars)) {}
        Vars fData;
    };
    sk_sp<RefCntVars> fVars;
};
```
