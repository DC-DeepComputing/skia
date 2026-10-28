---
name: skia/gpu/document-static-buffer-step-rate
description: When configuring static vertex buffer layouts in backend graphics pipelines, document the hardcoded vertex step rate assumption with an explanatory comment
rubric: readability
severity: nit
---

**Rule.** When configuring static buffer layouts in backend graphics pipelines,
document the hardcoded step rate (such as wgpu::VertexStepMode::Vertex,
VK_VERTEX_INPUT_RATE_VERTEX, or MTLVertexStepFunctionPerVertex) with an inline
comment. Clarify that static buffer data is currently assumed to always be
vertex-rate rather than dynamic or instance-rate.

**Why.** Graphite backend pipelines configure append buffer step rates
dynamically based on RenderStep properties (e.g., step->appendsVertices()),
while static buffer layouts hardcode vertex step rate. Leaving this assumption
undocumented creates ambiguity over whether instance-rate static data is
intentionally unsupported or omitted by mistake.

## Examples

Bad:

```cpp
auto& layout = vertexBufferLayouts[kStaticDataBufferIndex];
if (arrayStride) {
    layout.arrayStride = arrayStride;
    layout.stepMode = wgpu::VertexStepMode::Vertex;
    layout.attributeCount = staticDataAttributes.size();
    layout.attributes = staticDataAttributes.data();
}
```

Good:

```cpp
auto& layout = vertexBufferLayouts[kStaticDataBufferIndex];
if (arrayStride) {
    layout.arrayStride = arrayStride;
    // For now, static data is assumed to always be vertex rate.
    layout.stepMode = wgpu::VertexStepMode::Vertex;
    layout.attributeCount = staticDataAttributes.size();
    layout.attributes = staticDataAttributes.data();
}
```
