---
name: skia/gpu/document-caps-limit-units
description: When adding or updating hardware capability limit accessors in GPU Caps classes, document the exact unit or counting semantics when underlying graphics APIs differ
rubric: readability
severity: nit
---

**Rule.** When adding or modifying hardware or backend capability limit
accessors on `Caps` classes (such as `maxVaryings()`), document the exact unit
and counting semantics in accompanying doc comments. Explicitly clarify whether
the limit represents variable counts, scalar components, byte sizes, texels, or
alignment units whenever underlying graphics APIs (such as Vulkan, Dawn/WebGPU,
Metal, or OpenGL) specify pipeline constraints differently.

**Why.** Different graphics APIs and hardware platforms specify pipeline and
resource limits using disparate units (e.g., Vulkan and GL often count scalar
components like `maxVertexOutputComponents`, while WebGPU counts inter-stage
variables). Without explicit unit documentation, callers packing shader inputs,
sizing buffers, or validating pipeline limits risk underutilizing hardware or
triggering driver validation failures.

## Examples

Bad:

```cpp
class Caps {
public:
    // Bad: Ambiguous unit leaves callers unsure whether varyings are variables or scalar components
    int maxVaryings() const { return fMaxVaryings; }
};
```

Good:

```cpp
class Caps {
public:
    // Good: Documents exact counting semantics across differing backend API specifications
    // Returns the maximum number of varyings allowed in a render pipeline. Note that this is the
    // number of varying variables, not the total number of varying scalars.
    int maxVaryings() const { return fMaxVaryings; }
};
```
