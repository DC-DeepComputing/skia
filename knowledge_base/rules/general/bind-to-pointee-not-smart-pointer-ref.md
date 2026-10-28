---
name: skia/bind-to-pointee-not-smart-pointer-ref
description: When inspecting objects owned by smart pointers, bind local references or variables directly to the pointee (const T* or const T&) rather than const std::unique_ptr<T>&
rubric: readability
severity: nit
---

**Rule.** When inspecting, aliasing, or querying an object managed by a smart
pointer without transferring ownership, do not declare local variables or
references as const std::unique_ptr<T>& (or const sk_sp<T>&). Bind directly to
the underlying pointee type as const T* (via .get()) or const T& (via
dereferencing *).

**Why.** Binding a local reference to const std::unique_ptr<T>& unnecessarily
couples local inspection code to the container's storage and ownership
mechanism. Operating directly on const T* or const T& documents non-owning
access, eliminates redundant smart-pointer indirection, and allows callers to
work generically with the underlying object.

## Examples

Bad:

```cpp
const std::unique_ptr<Expression>& accessBase = arg.as<FieldAccess>().base();
SkASSERT(accessBase->is<VariableReference>());
```

Good:

```cpp
const Expression& accessBase = *arg.as<FieldAccess>().base();
SkASSERT(accessBase.is<VariableReference>());
```
