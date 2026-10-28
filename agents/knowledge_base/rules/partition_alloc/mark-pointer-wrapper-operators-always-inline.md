---
name: skia/partition_alloc/mark-pointer-wrapper-operators-always-inline
description: When implementing trivial accessors or overloaded operators in lightweight pointer and handle wrappers, annotate them with ALWAYS_INLINE and constexpr to guarantee zero-overhead inlining
rubric: safety
severity: should_fix
---

**Rule.** Annotate trivial accessors and overloaded operators (such as
operator[], operator*, operator->, and pointer arithmetic) in lightweight
pointer and handle wrapper types with ALWAYS_INLINE (such as SK_ALWAYS_INLINE or
SK_PA_ALWAYS_INLINE) and constexpr where applicable. Ensure that all wrapper
forwarding operations guarantee zero-overhead compiler inlining across
translation units.

**Why.** Pointer and handle wrapper classes (such as raw_ptr) are intended as
zero-overhead replacements for primitive pointers. Without an explicit
always-inline attribute, trivial operators and element indexing can fail to
inline under conservative compiler optimization heuristics, introducing
avoidable function call overhead into fundamental memory operations.

## Examples

Bad:

```cpp
// Bad: operator[] lacks inline annotation, risking function call overhead
template <typename Z, typename U = T>
U& operator[](Z delta) const {
    return wrapped_ptr_[delta];
}
```

Good:

```cpp
// Good: operator[] explicitly annotated to guarantee zero-overhead inlining
template <typename Z, typename U = T>
SK_PA_ALWAYS_INLINE constexpr U& operator[](Z delta) const {
    return wrapped_ptr_[delta];
}
```
