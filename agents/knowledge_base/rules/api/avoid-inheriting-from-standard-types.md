---
name: skia/api/avoid-inheriting-from-standard-types
description: When adding utility or domain methods to standard library types or containers, use free functions with type aliases or composition instead of inheritance
rubric: design
severity: should_fix
---

**Rule.** Do not inherit from C++ standard library types or containers (such as
std::vector, std::variant, std::string, or skstd::variant) to attach member
methods or domain utilities. Instead, use non-member free functions alongside
type aliases, or encapsulate the standard container as a private member using
composition.

**Why.** Standard library types lack virtual destructors and are not designed to
be base classes, making deletion via base pointers undefined behavior.
Subclassing standard containers merely to attach methods creates awkward hybrid
interfaces, risks object slicing, and breaks transparent interchangeability with
standard algorithms and vocabulary types.

## Examples

Bad:

```cpp
class SkPDFObject : public skstd::variant<bool, int32_t, SkScalar> {
public:
    void emit(SkWStream*) const;
};

class SkPDFArray : public std::vector<SkPDFObject> {
public:
    void emit(SkWStream*) const;
};
```

Good:

```cpp
using SkPDFObject = skstd::variant<bool, int32_t, SkScalar>;

class SkPDFArray {
public:
    // ...
private:
    std::vector<SkPDFObject> fArray;
};

void SkPDFEmit(const SkPDFObject&, SkWStream*);
void SkPDFEmit(const SkPDFArray&, SkWStream*);
```
