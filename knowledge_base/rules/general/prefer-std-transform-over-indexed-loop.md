---
name: skia/prefer-std-transform-over-indexed-loop
description: When converting or mapping elements from a source range to a destination container, prefer std::transform over manual indexed for loops
rubric: readability
severity: should_fix
---

**Rule.** Prefer standard algorithms like std::transform over manual indexed for
loops when converting, casting, or mapping elements from a source range to a
destination container. Pre-size the destination buffer using resize() and pass
data() or std::back_inserter to write the transformed elements.

**Why.** Standard algorithms express element-wise transformations declaratively,
eliminate manual index variables, bounds checking boilerplate, and off-by-one
errors. They clearly communicate intent and allow compiler optimizations over
contiguous memory.

## Examples

Bad:

```cpp
fColors4f.resize(src.size());
for (size_t i = 0; i < src.size(); ++i) {
    fColors4f[i] = SkColor4f::FromColor(src[i]);
}
```

Good:

```cpp
fColors4f.resize(src.size());
std::transform(src.begin(), src.end(), fColors4f.data(), [](SkColor c) {
    return SkColor4f::FromColor(c);
});
```
