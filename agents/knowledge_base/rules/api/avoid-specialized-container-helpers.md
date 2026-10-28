---
name: skia/api/avoid-specialized-container-helpers
description: When clearing or reducing the capacity of core containers like SkTArray, use existing methods like reserve_exact rather than adding specialized compound helpers
rubric: design
severity: should_fix
---

**Rule.** Do not expand core container classes (such as SkTArray, SkTDArray, or
SkTHashMap) with specialized compound helper methods when existing operations
suffice. When caller invariants guarantee a container is empty, use existing
methods such as reserve_exact() combined with debug assertions
(SkASSERT(container.empty())) to adjust capacity instead of introducing ad-hoc
container methods.

**Why.** Adding specialized or one-off compound methods bloats the API surface
of shared core containers, increases maintenance overhead, and couples generic
infrastructure to specific caller use cases. Relying on existing primitive
methods and caller invariants keeps container interfaces minimal and reusable.

## Examples

Bad:

```cpp
// In include/private/SkTArray.h:
void shrink_and_clear(int count) {
    this->changeSize(count);
    this->shrink_to_fit();
    this->changeSize(0);
}

// In caller:
fData.shrink_and_clear(halfCapacity);
```

Good:

```cpp
// In caller:
SkASSERT(fData.empty());
fData.reserve_exact(halfCapacity);
```
