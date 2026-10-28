---
name: skia/swap-all-member-variables
description: When implementing custom copy/move constructors, assignment operators, or swap functions, copy, move, or swap all member variables including conditionally compiled fields
rubric: correctness
severity: should_fix
---

**Rule.** When implementing or updating custom copy/move constructors,
assignment operators, or swap functions, ensure every non-static member
variable—including fields conditionally defined under preprocessor feature
guards (e.g., #if defined(...))—is properly copied, moved, or swapped. When
adding or modifying data members on a class or struct with custom state transfer
operations, update the corresponding constructor, operator=, or swap()
implementation to include the new fields.

**Why.** Omitting member variables from custom copy, move, or swap
implementations silently drops or corrupts state, leaving objects in an
inconsistent or partially uninitialized state. Conditionally compiled fields
(e.g., guarded by backend feature flags like SK_DAWN) are especially prone to
being overlooked on platforms where those macros are undefined.

## Examples

Bad:

```cpp
TestOptions& operator=(const TestOptions& other) {
    fContextOptions = other.fContextOptions;
    fOptionsPriv = other.fOptionsPriv;
    // Bug: omitted conditionally compiled SK_DAWN fields
    return *this;
}

void SkPath::swap(SkPath& that) {
    fPathData.swap(that.fPathData);
    std::swap(fFillType, that.fFillType);
    // Bug: omitted fIsFinite
}
```

Good:

```cpp
TestOptions& operator=(const TestOptions& other) {
    fContextOptions = other.fContextOptions;
    fOptionsPriv = other.fOptionsPriv;
#if defined(SK_DAWN)
    fDisableTintSymbolRenaming = other.fDisableTintSymbolRenaming;
#endif
    return *this;
}

void SkPath::swap(SkPath& that) {
    fPathData.swap(that.fPathData);
    std::swap(fFillType, that.fFillType);
    std::swap(fIsFinite, that.fIsFinite);
}
```
