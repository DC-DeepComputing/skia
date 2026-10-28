---
name: skia/api/prefer-as-over-cast-for-member-conversions
description: When defining member template methods that view, convert, or reinterpret an object or buffer as a target type, name them as<T>() rather than cast<T>()
rubric: readability
severity: nit
---

**Rule.** Name member template conversion and type-reinterpretation methods
as<T>() instead of cast<T>(). Reserve cast<T> for standalone free helper
functions (such as skvx::cast<T>).

**Why.** In Skia, as<T>() is the codebase-wide standard convention for member
functions that view, convert, or reinterpret an object or buffer as a target
type T (used extensively across AST nodes, JSON values, and buffer types).
Adhering to as<T>() ensures uniform API consistency across classes and keeps
cast<T> reserved for standalone free functions.

## Examples

Bad:

```cpp
class DataBuffer {
    template <typename T>
    const T* cast() const {
        return reinterpret_cast<const T*>(fSpan.data());
    }
};
```

Good:

```cpp
class DataBuffer {
    template <typename T>
    const T* as() const {
        return reinterpret_cast<const T*>(fSpan.data());
    }
};
```
