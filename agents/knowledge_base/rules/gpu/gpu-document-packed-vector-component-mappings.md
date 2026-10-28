---
name: skia/gpu/gpu-document-packed-vector-component-mappings
description: When declaring GPU vertex/instance attributes or packed vector fields, document component mappings and specify default fallback values for conditionally active attributes
rubric: readability
severity: nit
---

**Rule.** When packing multiple distinct semantic properties, coordinates, or
flags into compound vectors or GPU vertex/instance attributes (such as `SkV3`,
`SkV4`, or `float4`), explicitly document the component mapping (e.g., `.xy` and
`.z`) in accompanying comments. Additionally, when attributes or shader varyings
are only conditionally active or optional, document their expected default or
fallback values (such as all zeroes or no-op clip bounds) when the feature is
inactive.

**Why.** Compound vector types frequently pack disparate parameters to conserve
attribute slots, and GPU shaders often avoid branching by expecting inactive
attributes to contain benign sentinel or zero values. Documenting component
layouts and default values ensures CPU writer code and GPU shader code maintain
consistent assumptions, preventing attribute misalignment and subtle rendering
bugs.

## Examples

Bad:

```cpp
struct Vertex {
    // Unit circle space position and AA offset
    SkV3 fPosition;
};

// float4 inRoundCapPos - locations of the centers of the round caps.
{"inRoundCapPos", VertexAttribType::kFloat4, SkSLType::kFloat4},
```

Good:

```cpp
struct Vertex {
    // Unit circle local space position (.xy) and AA offset (.z)
    SkV3 fPosition;
};

// float4 inRoundCapPos - centers of round caps; all zeroes if not needed.
{"inRoundCapPos", VertexAttribType::kFloat4, SkSLType::kFloat4},
```
