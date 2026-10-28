---
name: skia/api/return-sk-sp-when-transferring-ownership
description: When returning newly allocated or ownership-transferring ref-counted objects from functions or getters, return sk_sp<T> by value rather than bare T* pointers
rubric: safety
severity: should_fix
---

**Rule.** When functions, factory methods, or getters hand off ownership or
transfer a reference to a ref-counted object, return sk_sp<T> by value rather
than a bare pointer (T*). Reserve returning bare T* strictly for borrowed,
non-owning references where the caller does not take ownership or manage object
lifetime.

**Why.** Returning bare pointers when transferring ownership requires callers to
manually adopt or unref the object, which is brittle and risks memory leaks or
dangling pointers. Returning sk_sp<T> by value makes ownership semantics
explicit in the type system and guarantees automatic lifetime management.

## Examples

Bad:

```cpp
// Bad: returning a bare pointer with an incremented or transferred ref count
virtual SkImageFilterCache* getImageFilterCache();

// Caller must manually wrap or adopt the naked pointer
sk_sp<SkImageFilterCache> cache(dst->getImageFilterCache());
```

Good:

```cpp
// Good: returning sk_sp makes ownership transfer explicit and self-documenting
virtual sk_sp<SkImageFilterCache> getImageFilterCache();

// Caller receives safe, automatic reference management
sk_sp<SkImageFilterCache> cache = dst->getImageFilterCache();
```
