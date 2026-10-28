---
name: skia/api/define-equality-on-composite-types
description: When comparing structs for equality or ordering, define comparison operators directly on the type rather than unpacking fields memberwise or passing ad-hoc lambdas
rubric: readability
severity: should_fix
---

**Rule.** Define comparison operators (operator==, operator<, operator>)
directly on structs and embedded value types rather than unpacking their fields
memberwise or passing ad-hoc equality lambdas to standard algorithms (such as
std::unique or std::find). When sorting or comparing records with multi-step
member comparisons, tie-breakers, or nullability checks, encapsulate the logic
within overloaded operators on the struct itself so callers and algorithms
compare instances directly; for function-local structs, implement operator== as
a member function.

**Why.** Inlining field-by-field comparisons, tie-breakers, or nullability
checks clutters callers and duplicates comparison logic. Defining comparison
operators directly on structs centralizes equality semantics with the type,
simplifies call sites and standard algorithm invocations, and prevents subtle
bugs when fields or tie-breakers change.

## Examples

Bad:

```cpp
// Variant 1: Enclosing struct unpacking inner struct fields memberwise
bool MasteringDisplayColorVolume::operator==(const MasteringDisplayColorVolume& o) const {
    return fPrimaries.fRX == o.fPrimaries.fRX && fPrimaries.fRY == o.fPrimaries.fRY &&
           fMaxLuminance == o.fMaxLuminance;
}

// Variant 2: Call site inlining multi-branch ordering and null checks
if (latest.layer) {
    if (curr.layer->fOrder > latest.layer->fOrder) { latest = curr; }
}

// Variant 3: Passing ad-hoc lambda to std::unique for a helper struct
struct EventTuple { int32_t y; int32_t x; };
auto eq = [](const EventTuple& a, const EventTuple& b) {
    return std::tie(a.y, a.x) == std::tie(b.y, b.x);
};
auto it = std::unique(events.begin(), events.end(), eq);
```

Good:

```cpp
// Variant 1: Embedded struct defines operator==; enclosing struct delegates
bool SkColorSpacePrimaries::operator==(const SkColorSpacePrimaries& o) const {
    return fRX == o.fRX && fRY == o.fRY;
}
bool MasteringDisplayColorVolume::operator==(const MasteringDisplayColorVolume& o) const {
    return fPrimaries == o.fPrimaries && fMaxLuminance == o.fMaxLuminance;
}

// Variant 2: Struct encapsulates ordering and nullability in operator>
bool Insertion::operator>(const Insertion& o) const {
    return !o.fLayer || (fLayer->fOrder > o.fLayer->fOrder);
}
if (curr > latest) { latest = curr; }

// Variant 3: Struct defines member operator== for default algorithm comparison
struct EventTuple {
    int32_t y; int32_t x;
    bool operator==(const EventTuple& r) const {
        return std::tie(this->y, this->x) == std::tie(r.y, r.x);
    }
};
auto it = std::unique(events.begin(), events.end());
```
