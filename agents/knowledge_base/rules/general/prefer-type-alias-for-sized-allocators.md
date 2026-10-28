---
name: skia/prefer-type-alias-for-sized-allocators
description: When specializing templated allocators or containers with fixed sizes, use a using type alias instead of a single-member wrapper class with forwarding accessors
rubric: design
severity: nit
---

**Rule.** Do not define a single-member wrapper class around SkSTArenaAlloc<N>
(or similar templated container or allocator types) merely to encapsulate a
fixed size parameter and forward an accessor. Define a 'using' type alias
directly (e.g., using MySizedArena = SkSTArenaAlloc<N>;) to preserve the named
size configuration while providing direct access to the allocator interface and
base class pointer conversions without boilerplate.

**Why.** Creating a dedicated wrapper class around a templated allocator or
container to encapsulate a constant size parameter introduces unnecessary
boilerplate, type indirection, and accessor forwarding methods. Because
SkSTArenaAlloc<N> already inherits publicly from SkArenaAlloc, a using type
alias directly exposes the allocator interface and allows passing its address as
an SkArenaAlloc* without intermediate accessors.

## Examples

Bad:

```cpp
// Over-engineered wrapper class just to bind a fixed size and forward an accessor
class SkBlitterSizedArena {
public:
    SkArenaAlloc* arena() { return &fAlloc; }

private:
    static constexpr size_t kStackMemory = 2736;
    SkSTArenaAlloc<kStackMemory> fAlloc;
};
```

Good:

```cpp
// Type alias directly names the sized allocator without boilerplate
using SkBlitterSizedArena = SkSTArenaAlloc<2736>;
```
