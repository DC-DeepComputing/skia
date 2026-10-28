---
name: skia/api/delegate-const-and-non-const-overloads
description: When overloading member functions on constness, ensure return types match in underlying type, omit redundant non-const overloads, and delegate between implementations
rubric: readability
severity: should_fix
---

**Rule.** Do not overload member functions on constness when return types differ
in underlying type rather than cv-qualification (such as returning T* vs
std::unique_ptr<T>&); use distinct method names or expose symmetrical accessors.
Omit redundant non-const member function overloads when a const method with
identical parameters and return type suffices. When const and non-const
overloads are required because return types differ in constness (such as const
T* vs T* or sk_sp<const T> vs sk_sp<T>), delegate between overloads (using
const_cast) rather than duplicating the implementation body.

**Why.** Const and non-const member overloads are expected to provide equivalent
access differing only in cv-qualification; returning different types (such as a
raw pointer vs a mutable smart-pointer reference) causes confusing overload
resolution based on caller constness. When return types and behavior are
identical, non-const overloads are redundant since const methods can be called
on non-const instances. When overloads genuinely differ in return constness,
duplicating logic risks divergence; delegating via const_cast keeps behavior in
sync.

## Examples

Bad:

```cpp
// Differing underlying return types on const overloads:
SymbolTable* symbolTable() const { return fSymbolTable.get(); }
std::unique_ptr<SymbolTable>& symbolTable() { return fSymbolTable; }

// Redundant non-const overload with identical implementation:
void setConvexity(SkPathConvexity c) { fPathData->setConvexity(c); }
void setConvexity(SkPathConvexity c) const { fPathData->setConvexity(c); }

// Duplicating logic across const and non-const overloads:
sk_sp<SkData> SkData::shareSubset(size_t offset, size_t length) {
    VALIDATE_SUBSET(this->size(), offset, length);
    // ... identical lookup and bounds checks ...
}
sk_sp<const SkData> SkData::shareSubset(size_t offset, size_t length) const {
    VALIDATE_SUBSET(this->size(), offset, length);
    // ... duplicated lookup and bounds checks ...
}
```

Good:

```cpp
// Matching types or distinct accessor names:
SymbolTable* symbolTable() const { return fSymbolTable.get(); }
SymbolTable* symbolTable() { return fSymbolTable.get(); }

// Single const method suffices for both const and non-const callers:
void setConvexity(SkPathConvexity c) const { fPathData->setConvexity(c); }

// Delegate between overloads using const_cast when return constness differs:
sk_sp<SkData> SkData::shareSubset(size_t offset, size_t length) {
    // ... canonical implementation ...
}
sk_sp<const SkData> SkData::shareSubset(size_t offset, size_t length) const {
    return const_cast<SkData*>(this)->shareSubset(offset, length);
}
```
