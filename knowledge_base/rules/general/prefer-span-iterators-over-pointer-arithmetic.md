---
name: skia/prefer-span-iterators-over-pointer-arithmetic
description: When traversing or bounding SkSpan or std::span, use begin() and end() with auto rather than raw pointer types, pointer arithmetic, or data_end
rubric: readability
severity: should_fix
---

**Rule.** Prefer standard container iterators (such as span.begin() and
span.end()) over manual pointer arithmetic (span.data() + span.size()) or custom
end-pointer helpers (such as data_end) when traversing, bounding, or obtaining
endpoints of SkSpan and contiguous collections. When capturing or binding span
iterators, declare variables with auto rather than binding to raw pointer types
(such as const T*).

**Why.** In std::span and standard containers, iterator types are
implementation-defined and not guaranteed to be raw pointers (e.g., in debug or
hardened standard libraries). Binding span iterators directly to raw pointer
types breaks portability and std::span migration, while manual pointer
arithmetic or custom helper functions clutter code with non-standard idioms.

## Examples

Bad:

```cpp
// Raw pointer arithmetic, helpers, or binding iterators to raw pointers:
template <typename T> T* data_end(SkSpan<T> span) {
    return span.data() + span.size();
}
const SkPathVerb* verbsEnd = data_end(verbSpan);
const SkScalar* conicWeights = src.conicWeights().end();
```

Good:

```cpp
// Use standard begin()/end() iterators deduced with auto:
auto verbsEnd = verbSpan.end();
auto conicWeights = src.conicWeights().end();
```
