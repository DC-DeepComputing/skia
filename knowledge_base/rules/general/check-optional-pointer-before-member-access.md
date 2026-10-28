---
name: skia/check-optional-pointer-before-member-access
description: 'When forwarding optional pointer arguments to underlying methods accepting nullptr, use ternary unwrapping (ptr ? &ptr->member : nullptr) instead of unconditional member access or branching if-else calls'
rubric: readability
severity: should_fix
---

**Rule.** When forwarding optional pointer arguments or extracting member
addresses in wrapper types, guard member access against nullptr to avoid
undefined behavior. When delegating to an underlying function that accepts
nullptr, prefer a concise ternary expression (`ptr ? &ptr->member : nullptr`)
instead of branching with an `if (!ptr)` check that duplicates call sites.

**Why.** In C++, `ptr->member` is equivalent to `(*ptr).member`, which
dereferences `ptr` and invokes undefined behavior if `ptr` is `nullptr`—even
when taking its address (`&ptr->member`). Using a ternary expression safely
checks for `nullptr` and passes the underlying member address or `nullptr` in a
single call, avoiding both undefined behavior and redundant branching
boilerplate.

## Examples

Bad:

```cpp
// Bad: Unconditional member access invokes undefined behavior if caller passes nullptr
bool invert(LayerSpace<SkMatrix>* inverse) const {
    return fData.invert(&inverse->fData);
}

// Bad: Redundant branching duplicates the underlying call site
bool invert(LayerSpace<SkMatrix>* inverse) const {
    if (!inverse) {
        return fData.invert(nullptr);
    }
    return fData.invert(&inverse->fData);
}
```

Good:

```cpp
// Good: Ternary expression guards against nullptr without duplicating the call
bool invert(LayerSpace<SkMatrix>* inverse) const {
    return fData.invert(inverse ? &inverse->fData : nullptr);
}
```
