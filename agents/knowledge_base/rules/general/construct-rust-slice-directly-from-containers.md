---
name: skia/construct-rust-slice-directly-from-containers
description: When constructing rust::Slice from contiguous containers like std::array, std::vector, or SkSpan, pass the container directly instead of separate data() and size()
rubric: readability
severity: should_fix
---

**Rule.** When constructing a rust::Slice<T> from a contiguous container (such
as std::array, std::vector, or SkSpan), use the container-taking constructor
rust::Slice<T>(container) rather than passing separate .data() and .size()
arguments.

**Why.** Constructing slices directly from containers avoids passing raw pointer
and size pairs, reducing the risk of buffer bounds mismatches and producing
cleaner, idiomatic FFI code under spanification guidelines.

## Examples

Bad:

```cpp
std::array<float, 10> mdcvArray;
// ...
rust::Slice<const float> mdcvSlice(mdcvArray.data(), mdcvArray.size());
```

Good:

```cpp
std::array<float, 10> mdcvArray;
// ...
rust::Slice<const float> mdcvSlice(mdcvArray);
```
