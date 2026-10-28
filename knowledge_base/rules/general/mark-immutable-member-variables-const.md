---
name: skia/mark-immutable-member-variables-const
description: When declaring class member variables fixed at construction, mark immutable primitives, value members, containers, and pointers const, but avoid const on options structs or sk_sp
rubric: readability
severity: should_fix
---

**Rule.** Mark class member variables (including primitives, raw pointers,
containers like std::vector, and value objects like SkString) const and
initialize them in constructor initializer lists when their values are fixed at
construction, provided the class does not require copy or move assignment
operators. For raw pointer members where the pointee is mutated but the pointer
reference is fixed, mark the pointer itself const (e.g., T* const fMember).
Conversely, do not declare fields const in plain data or options structs (to
preserve assignment and customization), and avoid declaring ref-counted smart
pointers const (e.g., const sk_sp<T>), which inhibits move semantics and
ref-count transfer without qualifying the pointee.

**Why.** Declaring invariant member variables (primitives, raw pointers,
containers, and value objects) const enforces immutability at compile time,
prevents accidental mutation, and documents class invariants. Conversely, const
fields in options structs disable compiler-generated copy- and move-assignment
operators. For smart pointers like sk_sp<T>, const qualification disables move
construction and assignment (causing moves to silently fall back to atomic
ref-count copies) while qualifying only the smart pointer handle rather than the
pointee.

## Examples

Bad:

```cpp
class DrawAtlas {
    ...
    int fMaxPages;
    GenerationCounter* fGenerationCounter;
};

// Container member left mutable despite being fixed at construction:
class EventQueue {
    const std::vector<CompactEvent> fEvents;
    std::vector<Segment> fSegmentStorage;
};

// Value member left mutable despite being fixed at construction like sibling fields:
SkString fFamilyName;
const SkString fPathName;

// Mutable fields assigned post-construction rather than initialized const:
struct RPOps {
    int fSrcBpp;
    int fDstBpp;
};
auto ops = std::make_unique<RPOps>();
ops->fSrcBpp = SkColorTypeBytesPerPixel(srcColorType);

// Plain data or options struct with const fields disabling assignment:
struct BufferOptions {
    const uint32_t fVertexBufferMinSize;
    const uint32_t fVertexBufferMaxSize;
};

// Const smart pointers disable move semantics and only qualify the handle:
class PrecompileColorSpaceXformColorFilter : public PrecompileColorFilter {
    ...
private:
    const sk_sp<SkColorSpace> fSrc;
    const sk_sp<SkColorSpace> fDst;
};
```

Good:

```cpp
class DrawAtlas {
    ...
    const int fMaxPages;
    GenerationCounter* const fGenerationCounter;
};

// Containers and value members marked const consistently when fixed at construction:
class EventQueue {
    const std::vector<CompactEvent> fEvents;
    const std::vector<Segment> fSegmentStorage;
};
const SkString fFamilyName;
const SkString fPathName;

// Immutable fields declared const and initialized in constructor:
class RPOps {
public:
    RPOps(int srcBpp, int dstBpp) : fSrcBpp(srcBpp), fDstBpp(dstBpp) {}
    const int fSrcBpp;
    const int fDstBpp;
};
auto ops = std::make_unique<RPOps>(SkColorTypeBytesPerPixel(srcColorType), ...);

// Keep options struct fields mutable; pass by const-ref or value:
struct BufferOptions {
    uint32_t fVertexBufferMinSize = 16 << 10;
};
void init(const BufferOptions& options);

// Leave smart pointers non-const to permit move semantics and ref transfer:
class PrecompileColorSpaceXformColorFilter : public PrecompileColorFilter {
    ...
private:
    sk_sp<SkColorSpace> fSrc;
    sk_sp<SkColorSpace> fDst;
};
```
