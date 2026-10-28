---
name: skia/use-empty-braces-for-array-zero-initialization
description: When zero-initializing fixed-size arrays or aggregate structs, use empty braces `{}` rather than explicitly repeating zero literals for every element
rubric: readability
severity: nit
---

**Rule.** When zero-initializing fixed-size arrays, multi-dimensional buffers,
or aggregate structs, use empty braces `{}` (or `= {}`) instead of explicitly
enumerating zero literals or nested zero initializers for each element. Rely on
C++ value-initialization semantics to zero all scalar members and
default-construct elements automatically.

**Why.** In C++, empty braces `= {}` value-initialize all elements, zeroing
fundamental types and invoking default constructors for sub-objects. Explicitly
repeating zero literals (`{0, 0, 0}` or `{{0, 0}, {0, 0}}`) adds visual noise,
increases maintenance burden, and is fragile if array bounds or member counts
change.

## Examples

Bad:

```cpp
SkPoint points[3] = {{0, 0}, {0, 0}, {0, 0}};
float matrix[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
```

Good:

```cpp
SkPoint points[3] = {};
float matrix[4][4] = {};
```
