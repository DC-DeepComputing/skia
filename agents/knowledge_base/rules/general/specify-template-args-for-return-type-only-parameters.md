---
name: skia/specify-template-args-for-return-type-only-parameters
description: When calling function templates where type parameters appear only in return types, explicitly specify template type arguments at call sites
rubric: correctness
severity: blocking
---

**Rule.** Explicitly specify template type arguments when calling function
templates whose parameters cannot be deduced from function arguments, such as
allocator, factory, or conversion methods where the type parameter appears only
in the return type. Review uninstantiated template definitions carefully, as
omitted template arguments may escape compiler diagnostics until instantiation.

**Why.** C++ function template argument deduction operates solely on function
call arguments, not assignment destinations or return contexts. In
uninstantiated template definitions, missing template arguments may not be
diagnosed by the compiler until the template is instantiated, allowing broken
call sites to escape initial builds.

## Examples

Bad:

```cpp
template <typename T>
SkSpan<T> makePODSpan(SkSpan<const T> s) {
    // Missing <T> cannot be deduced from the integer size argument.
    T* result = this->makePODArray(SkTo<int>(s.size()));
    return SkSpan<T>(result, s.size());
}
```

Good:

```cpp
template <typename T>
SkSpan<T> makePODSpan(SkSpan<const T> s) {
    // Explicitly specify <T> since makePODArray type parameter is only in return type.
    T* result = this->makePODArray<T>(SkTo<int>(s.size()));
    return SkSpan<T>(result, s.size());
}
```
