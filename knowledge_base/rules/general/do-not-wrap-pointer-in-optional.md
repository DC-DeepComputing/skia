---
name: skia/do-not-wrap-pointer-in-optional
description: When returning, passing, or storing nullable values, do not wrap raw pointers, smart pointers (sk_sp), or composite types containing them in std::optional
rubric: design
severity: should_fix
---

**Rule.** Do not wrap raw pointers (T*), smart pointers (such as sk_sp<T> or
std::unique_ptr<T>), or composite types containing them (such as std::tuple,
std::pair, or structs) in std::optional in function return types, parameters, or
member variables. Use the pointer or composite type directly, relying on nullptr
or an empty smart pointer state to represent absence or failure.

**Why.** Pointers and smart pointers already encode absence or failure via
nullptr or an empty state. Wrapping them directly or wrapping composite types
containing them in std::optional introduces redundant nested nullability (e.g.
std::nullopt vs nullptr), increases template nesting complexity, and forces
callers to perform redundant checks for both optional engagement and pointer
validity.

## Examples

Bad:

```cpp
// Bad: Wrapping raw or smart pointers, or tuples containing them, in std::optional.
std::tuple<CompressedPaintersOrder, std::optional<Layer*>> updateClipStateForDraw(
        const Clip& clip,
        const ElementList& effectiveElements,
        const BoundsManager*,
        PaintersDepth z);

std::optional<sk_sp<Buffer>> getStaticVertexBuffer();

using ClipResult = std::optional<std::tuple<sk_sp<TextureProxy>, SkIPoint>>;
ClipResult findOrCreateEntry(uint32_t stackRecordID,
                             const ClipStack::ElementList* elements,
                             SkIRect maskDeviceBounds);
```

Good:

```cpp
// Good: Return raw/smart pointers or tuples directly and rely on null/empty pointer state.
std::tuple<CompressedPaintersOrder, Layer*> updateClipStateForDraw(
        const Clip& clip,
        const ElementList& effectiveElements,
        const BoundsManager*,
        PaintersDepth z);

sk_sp<Buffer> getStaticVertexBuffer();

using ClipResult = std::tuple<sk_sp<TextureProxy>, SkIPoint>;
ClipResult findOrCreateEntry(uint32_t stackRecordID,
                             const ClipStack::ElementList* elements,
                             SkIRect maskDeviceBounds);
```
