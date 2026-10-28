---
name: skia/avoid-redundant-non-template-declarations
description: When a class provides a member function template accommodating parameter variations across platforms, do not declare redundant non-template member overloads that the template subsumes
rubric: readability
severity: nit
---

**Rule.** Do not declare redundant non-template member functions when an
unconstrained member function template in the class already subsumes that
parameter signature. Rely on the member template to handle calls across
platforms and API revisions rather than declaring platform-specific non-template
overloads.

**Why.** A separate non-template declaration is redundant when an unconstrained
member template already handles the signature. Furthermore, because C++ overload
resolution prefers non-template functions over function templates for exact
matches, declaring a non-template overload without providing its definition
causes link-time undefined symbol errors on matching platforms.

## Examples

Bad:

```cpp
#if defined(__EMSCRIPTEN__)
    void mapCallback(WGPUBufferMapAsyncStatus status, const char* message);
#endif

    template <typename StatusT, typename MessageT>
    void mapCallback(StatusT status, MessageT message);
```

Good:

```cpp
    // The unconstrained member template accommodates both native Dawn and Emscripten
    // callback signatures without needing redundant platform-specific declarations.
    template <typename StatusT, typename MessageT>
    void mapCallback(StatusT status, MessageT message);
```
