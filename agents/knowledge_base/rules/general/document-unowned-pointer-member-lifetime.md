---
name: skia/document-unowned-pointer-member-lifetime
description: When declaring or storing unowned raw pointers or references in classes or setters, document which entity owns the pointee and caller lifetime requirements
rubric: readability
severity: should_fix
---

**Rule.** When a class, struct, adapter, or setter method stores or retains an
unowned raw pointer, buffer, or reference (such as `const T& fMember`, `T*
fMember`, or a setter accepting `const char*` or `const void*` without copying),
explicitly document the lifetime contract and ownership on the declaration. For
member fields pointing to externally managed objects, specify which entity (such
as a SymbolTable, Arena, or Cache) owns the pointee and under what conditions it
remains valid. For caller-borrowed inputs, state clearly that data is not copied
and that the caller must ensure the referenced object remains valid and outlives
the receiving instance, and preserve these lifetime comments across
refactorings.

**Why.** Non-owning pointers and references rely on external or caller-managed
lifetimes, creating dangling pointer and use-after-free hazards if the
referenced instance or buffer is destroyed first. Clearly documenting which
component owns the pointed-to object and recording caller lifetime requirements
ensures maintainers uphold ownership invariants, verify lifetime safety, and
avoid accidental lifetime mismatches.

## Examples

Bad:

```cpp
// Storing unowned pointer without documenting owner/validity
class Variable {
private:
    const std::string* fMangledName = nullptr;
};

// Adapter class storing unowned stream pointer without caller lifetime requirement
class SkStreamAdapter {
public:
    explicit SkStreamAdapter(SkStream* stream) : fStream(stream) {}
private:
    SkStream* fStream;
};

// Setter retaining raw pointer without documenting caller ownership
void setAttrValue(const Node* node, Attr* attr, const char* value);

// Refactored struct dropping member lifetime documentation
class DrawParams {
    const Transform& fTransform;
    Geometry         fGeometry;
};
```

Good:

```cpp
// Explicitly documents which external entity owns the pointee
class Variable {
private:
    // When non-null, `fMangledName` is owned by the SymbolTable.
    const std::string* fMangledName = nullptr;
};

// Caller must guarantee that `stream` outlives the SkStreamAdapter.
class SkStreamAdapter {
public:
    explicit SkStreamAdapter(SkStream* stream) : fStream(stream) {}
private:
    SkStream* fStream;
};

// Value string is not copied; its lifetime must be managed by the caller.
void setAttrValue(const Node* node, Attr* attr, const char* value);

// Preserve lifetime invariants when refactoring non-owning member declarations
class DrawParams {
    const Transform& fTransform; // Lifetime of the transform must outlive the geometry
    Geometry         fGeometry;
};
```
