---
name: skia/gpu/centralize-alignment-in-buffer-reserve
description: When designing buffer appender or allocator hierarchies, centralize data alignment and padding calculations inside base reserve() methods rather than requiring caller or subclass pre-alignment
rubric: design
severity: should_fix
---

**Rule.** In buffer appender and allocator class hierarchies, enforce
data-specific alignment and padding constraints internally within the base
reservation method (such as reserve()). Do not require callers (like append())
or derived classes to pre-align reservation counts or define redundant wrapper
overrides of reserve().

**Why.** Centralizing alignment and padding adjustments inside the base
reservation function prevents duplicated alignment logic across callers, avoids
fragile derived-class overrides that exist solely to wrap reserve(), and ensures
consistent buffer reservation behavior whether space is requested via appending
helpers or direct reservation calls.

## Examples

Bad:

```cpp
// Caller pre-aligns count and derived class overrides reserve():
VertexWriter append(unsigned int count) {
    uint32_t alignedCount = fAppendVerts ? SkAlign4(count) : count;
    this->reserve(alignedCount);
    ...
}
class Vertices : private Appender {
    void reserve(unsigned int count) {
        DrawWriter::Appender::reserve(SkAlign4(count));
    }
};
```

Good:

```cpp
// Base reserve() adjusts alignment internally; callers and derived classes pass raw count:
void reserve(unsigned int count) {
    const uint32_t alignedCount = count + (fAppendVerts ? countDiff : 0);
    ...
}
VertexWriter append(unsigned int count) {
    this->reserve(count);
    ...
}
class Vertices : private Appender {
    using Appender::reserve;
};
```
