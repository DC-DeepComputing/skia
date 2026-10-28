---
name: skia/mark-query-methods-and-read-only-pointers-const
description: When declaring non-mutating accessors, query methods, operators, or pointer parameters, mark methods const, return pointers to const (sk_sp<const T>), and use const T*
rubric: readability
severity: should_fix
---

**Rule.** Mark member functions, accessors, query methods, predicates (such as
is*, has*, needs*), and member comparison operators as const whenever they only
read member state and do not modify the object. When returning pointers or smart
pointers (such as sk_sp<const T> or const T*) from const methods or accessors,
return pointers to const pointees to preserve logical constness. In function
declarations and definitions, when passing pointers to unmutated objects,
optional configurations, or input descriptors, declare the parameter as
pointer-to-const (const T*) rather than a mutable pointer (T*).

**Why.** Const-qualifying non-mutating methods and comparison operators ensures
they can be invoked on const instances and references (such as const Iter& in
standard algorithms), documents lack of side effects, and catches unintended
mutations at compile time. Returning pointers or smart pointers to const
pointees (such as sk_sp<const T> or const T*) prevents callers from mutating
internal state through a const receiver, preserving logical constness.
Qualifying read-only pointer parameters as pointer-to-const enforces
const-correctness, makes caller mutation intentions explicit, and allows callers
holding const instances to pass their data without casting.

## Examples

Bad:

```cpp
// Non-const member comparison operator on iterator or value type
struct Iter {
    bool operator!=(const Iter& b) { return fVerb != b.fVerb; }
};

// Non-const query method or accessor that only reads object state
bool willVertexOverflow(size_t count, size_t dataStride, size_t alignStride);
bool needsStageOutputStruct();
skia_private::TArray<uint32_t> getBufferInfo() { return fInfo; }

// Returning mutable smart pointer or pointer from non-const or const accessor
sk_sp<skresources::ImageAsset> getImageAsset() { return fImageAsset; }

// Non-const pointer parameters for unmutated inputs or descriptors
bool willBufferOverflow(BufferInfo* info, uint32_t requiredBytes, uint32_t requiredAlignment);
SpvId writeFunctionCallArgument(const FunctionCall& call,
                                Analysis::SpecializedParameters* specializedParams);
```

Good:

```cpp
// Const member comparison operator safe for const instances and references
struct Iter {
    bool operator!=(const Iter& b) const { return fVerb != b.fVerb; }
};

// Const query method and accessor safe for const instances
bool willVertexOverflow(size_t count, size_t dataStride, size_t alignStride) const;
bool needsStageOutputStruct() const;
skia_private::TArray<uint32_t> getBufferInfo() const { return fInfo; }

// Const accessor returning smart pointer to const preserving logical constness
sk_sp<const skresources::ImageAsset> getImageAsset() const { return fImageAsset; }

// Pointer-to-const parameters enforcing immutability of read-only inputs
bool willBufferOffsetOverflow(const BufferInfo* info,
                              uint32_t requiredBytes,
                              uint32_t requiredAlignment) const;
SpvId writeFunctionCallArgument(const FunctionCall& call,
                                const Analysis::SpecializedParameters* specializedParams);
```
