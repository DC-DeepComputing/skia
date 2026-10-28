---
name: skia/gpu/eliminate-outset-vertex-attributes-without-curved-corners
description: When specializing GPU geometry shaders or RenderSteps to eliminate curved corners or stroking, omit dedicated outset-scaling vertex attributes by encoding outset presence in non-zero normals
rubric: safety
severity: should_fix
---

**Rule.** When specializing a GPU geometry shader or RenderStep from curved or
stroked shapes (such as analytic rounded rects) to flat geometry without rounded
corners, eliminate dedicated outset-scaling vertex attributes (such as
normalScale). Assign zero-length normal vectors (float2(0.0)) to non-outset
vertices, and test the normal vector directly (e.g., any(notEqual(normal,
float2(0.0)))) in the vertex shader to identify outset vertices.

**Why.** In general rendering pipelines with curved boundaries, interior
non-outset vertices often require valid normal vectors to calculate local curve
positions, necessitating a separate scaling attribute to distinguish outsets.
For specialized flat geometry without curved corners, non-outset vertices do not
need normals; encoding the outset state in the normal vector eliminates a vertex
attribute slot and reduces vertex buffer memory bandwidth.

## Examples

Bad:

```cpp
// Retaining separate normalScale attribute when non-outset vertices do not need normals:
float4 specialized_vertex_fn(float2 position,
                             float2 normal,
                             float normalScale,
                             float depth,
                             float3x3 localToDevice) {
    ...
    if (normalScale > 0.0 && devPos.z > 0.0) {
        // Outset calculation
    }
}
```

Good:

```cpp
// Non-outset vertices supply float2(0.0), eliminating the dedicated scaling attribute:
float4 specialized_vertex_fn(float2 position,
                             float2 normal,
                             float depth,
                             float3x3 localToDevice) {
    ...
    if (any(notEqual(normal, float2(0.0))) && devPos.z > 0.0) {
        // Outset calculation
    }
}
```
