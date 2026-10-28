---
name: skia/auto-non-type-template-params
description: When declaring templates with non-type parameters like function pointers, use auto instead of hardcoded function signatures or paired type parameters requiring decltype
rubric: readability
severity: should_fix
---

**Rule.** When declaring class templates, template aliases, or function wrappers
that accept non-type template parameters (such as function pointers or
deleters), use auto for the non-type parameter (e.g., template <auto Fn>). Avoid
hardcoding specific function pointer signatures like void(*)(T*) that
unnecessarily restrict argument types, and avoid requiring a separate type
parameter and non-type parameter pair (e.g., typename P, P* p) that forces
callers to pass redundant decltype(...) arguments.

**Why.** C++17 allows non-type template parameters to deduce their type directly
via auto. Using auto accommodates varying function signatures and calling
conventions without hardcoding brittle pointer types, while eliminating
repetitive decltype(...) boilerplate and extra type parameters at instantiation
sites.

## Examples

Bad:

```cpp
// Bad: Hardcodes a specific function pointer signature, restricting deleters
template <class T, void(*P)(T*)>
using Resource = std::unique_ptr<T, SkFunctionWrapper<P>>;
using HBBlob = Resource<hb_blob_t, &hb_blob_destroy>;

// Bad: Requires callers to redundantly pass the function type via decltype
template <typename T, typename P, P* p>
using Resource2 = std::unique_ptr<T, SkFunctionWrapper<p>>;
using ICUBrk = Resource2<UBreakIterator, decltype(ubrk_close), ubrk_close>;
```

Good:

```cpp
// Good: Uses auto to deduce any compatible function pointer signature
template <class T, auto p>
using Resource = std::unique_ptr<T, SkFunctionWrapper<p>>;
using HBBlob = Resource<hb_blob_t, hb_blob_destroy>;
using ICUBrk = Resource<UBreakIterator, ubrk_close>;
```
