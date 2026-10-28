---
name: skia/gpu/gpu-name-vertex-attribute-helpers-by-granularity
description: When defining helpers that return vertex attribute arrays in GPU render steps, name them after the attributes and granularity rather than ambiguous single-item getters
rubric: readability
severity: nit
---

**Rule.** When generating vertex attribute arrays or geometry buffers in GPU
render steps, name helper functions after the specific attributes and structural
granularity they return (e.g., get_per_corner_vertex_attrs rather than
get_vertex_corner). Avoid ambiguous names that sound like getters for a single
vertex coordinate, corner ID, or scalar index.

**Why.** Helper functions generating arrays of vertex attributes produce
structured multi-vertex collections. Ambiguous names like get_vertex_corner
sound like accessors for an individual corner index or vertex coordinate,
obscuring the return type and data granularity at call sites.

## Examples

Bad:

```cpp
template <uint32_t kCornerID>
constexpr std::array<Vertex, kCornerVertexCount> get_vertex_corner() {
    return {{
        { kCornerID, {1.0f, 0.0f} },
        // ...
    }};
}
```

Good:

```cpp
template <uint32_t kCornerID>
constexpr std::array<Vertex, kCornerVertexCount> get_per_corner_vertex_attrs() {
    return {{
        { kCornerID, {1.0f, 0.0f} },
        // ...
    }};
}
```
