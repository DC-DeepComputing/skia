---
name: skia/encapsulate-composite-validity-in-operator-bool
description: When checking composite structs or handle types with multi-field validity invariants, define explicit operator bool() const instead of repeating compound member checks
rubric: readability
severity: should_fix
---

**Rule.** Define an explicit operator bool() const on composite structs, state
records, or handle types whose validity or presence depends on multiple member
pointers or fields. Do not require callers to repeatedly inspect internal
members with compound boolean checks (such as f.a && f.b or !f.a && !f.b) across
assertions and conditional guards.

**Why.** Encapsulating multi-field validity checks in an explicit operator
bool() centralizes the type's validity invariant in a single location, avoids
brittle duplication of compound pointer checks across call sites, and simplifies
assertions and if-conditions. Marking the conversion explicit prevents
accidental implicit conversions to integer or arithmetic types while still
supporting contextual boolean evaluation.

## Examples

Bad:

```cpp
struct Insertion {
    Layer* fLayer = nullptr;
    BindingWrapper* fWrapper = nullptr;
};

// Callers repeatedly write compound member checks:
SkASSERT(!fInsertion.fLayer && !fInsertion.fWrapper);
if (fInsertion.fLayer && fInsertion.fWrapper) {
    applyInsertion(fInsertion);
}
```

Good:

```cpp
struct Insertion {
    Layer* fLayer = nullptr;
    BindingWrapper* fWrapper = nullptr;

    explicit operator bool() const {
        return (fLayer != nullptr) && (fWrapper != nullptr);
    }
};

// Callers test validity contextually:
SkASSERT(!fInsertion);
if (fInsertion) {
    applyInsertion(fInsertion);
}
```
