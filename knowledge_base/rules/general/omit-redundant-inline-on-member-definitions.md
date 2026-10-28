---
name: skia/omit-redundant-inline-on-member-definitions
description: When defining member functions or operators directly inside a class or struct declaration, omit the redundant 'inline' specifier
rubric: readability
severity: nit
---

**Rule.** Do not add the redundant 'inline' specifier to member functions,
methods, constructors, or operators defined directly inside a class or struct
declaration. Rely on standard C++ implicit inline linkage for in-class member
definitions.

**Why.** In C++, any member function or operator defined directly within a class
or struct declaration is implicitly inline and granted inline linkage by the
compiler. Explicitly specifying 'inline' on in-class member definitions adds
redundant syntactic clutter without altering compiler behavior or linkage.

## Examples

Bad:

```cpp
struct SkPDFIndirectReference {
    int fValue = -1;
    explicit operator bool() const { return fValue != -1; }

    inline bool operator==(SkPDFIndirectReference v) const {
        return fValue == v.fValue;
    }
};
```

Good:

```cpp
struct SkPDFIndirectReference {
    int fValue = -1;
    explicit operator bool() const { return fValue != -1; }

    bool operator==(SkPDFIndirectReference v) const {
        return fValue == v.fValue;
    }
};
```
