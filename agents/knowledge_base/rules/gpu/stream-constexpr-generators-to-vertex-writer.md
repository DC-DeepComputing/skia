---
name: skia/gpu/stream-constexpr-generators-to-vertex-writer
description: When writing static vertex or index buffers with VertexWriter, stream constexpr generator calls directly into the writer with inline comments rather than declaring intermediate static constexpr variables
rubric: readability
severity: should_fix
---

**Rule.** When writing static vertex, instance, or index data using VertexWriter
(or BufferWriter), stream constexpr generator function calls directly into the
writer with inline comments rather than declaring single-use intermediate static
constexpr variables. Annotate each streamed call with a concise inline comment
(e.g. // TL, // TR) to document semantic meaning without intermediate variable
boilerplate.

**Why.** Declaring separate single-use static constexpr intermediate variables
solely to stream them once into a buffer writer introduces redundant boilerplate
and visual clutter. Streaming generator function calls directly while annotating
each entry with inline comments keeps buffer-writing routines compact and
readable without sacrificing clarity.

## Examples

Bad:

```cpp
static void write_vertex_buffer(VertexWriter writer) {
    static constexpr std::array<Vertex, kCornerVertexCount> kCornerTL = get_vertex_corner<0>();
    static constexpr std::array<Vertex, kCornerVertexCount> kCornerTR = get_vertex_corner<1>();
    static constexpr std::array<Vertex, kCornerVertexCount> kCornerBR = get_vertex_corner<2>();
    static constexpr std::array<Vertex, kCornerVertexCount> kCornerBL = get_vertex_corner<3>();

    if (writer) {
        writer << kCornerTL
               << kCornerTR
               << kCornerBR
               << kCornerBL;
    }
}
```

Good:

```cpp
static void write_vertex_buffer(VertexWriter writer) {
    if (writer) {
        writer << get_vertex_corner<0>()  // TL
               << get_vertex_corner<1>()  // TR
               << get_vertex_corner<2>()  // BR
               << get_vertex_corner<3>(); // BL
    }
}
```
