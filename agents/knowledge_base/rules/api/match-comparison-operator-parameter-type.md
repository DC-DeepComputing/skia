---
name: skia/api/match-comparison-operator-parameter-type
description: When defining comparison operators (operator==, operator!=) on structs or wrapper types, verify parameter types match the enclosing struct rather than a copied peer type
rubric: correctness
severity: should_fix
---

**Rule.** When defining member or non-member comparison operators (such as
operator==, operator!=, or operator<=>) on structs, ID types, or wrapper
classes, ensure the parameter types match the enclosing type rather than an
accidentally copied peer type. Verify that comparison signatures do not
inadvertently allow heterogeneous comparisons or break homogeneous equality
checks.

**Why.** Copy-pasting comparison operator boilerplate between sibling structs or
handle types can leave parameters typed as a neighboring struct. Because
heterogeneous comparison signatures are valid C++, the compiler will not flag
the mismatch, leading to broken homogeneous comparisons between instances of the
struct and allowing unintended comparisons across distinct strong types.

## Examples

Bad:

```cpp
struct SkPDFParentTreeKey {
    int fValue = -1;
    // Bad: Parameter type copied from SkPDFIndirectReference
    bool operator==(SkPDFIndirectReference v) const {
        return fValue == v.fValue;
    }
    bool operator!=(SkPDFIndirectReference v) const {
        return fValue != v.fValue;
    }
};
```

Good:

```cpp
struct SkPDFParentTreeKey {
    int fValue = -1;
    // Good: Parameter type matches the enclosing struct
    bool operator==(SkPDFParentTreeKey v) const {
        return fValue == v.fValue;
    }
    bool operator!=(SkPDFParentTreeKey v) const {
        return fValue != v.fValue;
    }
};
```
