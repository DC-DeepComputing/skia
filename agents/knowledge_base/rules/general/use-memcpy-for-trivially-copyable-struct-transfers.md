---
name: skia/use-memcpy-for-trivially-copyable-struct-transfers
description: When transferring trivially copyable struct data to or from raw memory buffers, use memcpy instead of pointer type-punning or element-wise copying
rubric: readability
severity: should_fix
---

**Rule.** When transferring trivially copyable struct data (such as SkColor4f or
vector types) to or from raw memory buffers, WASM pointers, or FFI memory, use
std::memcpy rather than manual element-by-element assignment or pointer
type-punning. Avoid assigning members individually across branching code paths
when the full struct representation can be populated and copied directly.

**Why.** Direct pointer type-punning (e.g. casting raw memory directly to struct
pointers) violates C++ strict aliasing rules and causes undefined behavior,
while manual element-wise assignment is verbose and error-prone across branching
paths. std::memcpy is well-defined under C++ aliasing rules for trivially
copyable types and compiles to efficient vector stores or register moves.

## Examples

Bad:

```cpp
// Bad: Element-by-element assignment to raw buffer across branching paths
float* fourFloats = reinterpret_cast<float*>(outPtr);
if (auto c = fSlotMgr->getColorSlot(SkString(slotID))) {
    auto c4f = SkColor4f::FromColor(*c);
    fourFloats[0] = c4f.fR;
    fourFloats[1] = c4f.fG;
    fourFloats[2] = c4f.fB;
    fourFloats[3] = c4f.fA;
} else {
    fourFloats[0] = -1;
    fourFloats[1] = -1;
    fourFloats[2] = -1;
    fourFloats[3] = -1;
}
```

Good:

```cpp
// Good: Populate struct and copy via std::memcpy
SkColor4f c4f;
if (auto c = fSlotMgr->getColorSlot(SkString(slotID))) {
    c4f = SkColor4f::FromColor(*c);
} else {
    c4f = {-1, -1, -1, -1};
}
std::memcpy(reinterpret_cast<float*>(outPtr), &c4f, sizeof(c4f));
```
